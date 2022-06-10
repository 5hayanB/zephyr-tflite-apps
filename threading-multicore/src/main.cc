#include <zephyr/zephyr.h>
#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_error_reporter.h>
#include <tensorflow/lite/schema/schema_generated.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/micro/micro_mutable_op_resolver.h>
#include "model.h"
#include "car1.h"
#include "car2.h"
#include "car3.h"
#include "car4.h"
#include "car5.h"
#include "person1.h"
#include "person2.h"
#include "person3.h"
#include "person4.h"
#include "person5.h"
#include "person6.h"
#include "person7.h"
#include "person8.h"
#include "person9.h"
#include "person10.h"
#include "space1.h"
#include "space2.h"
#include "space3.h"
#include "space4.h"
#include "monkey.h"
#include "mask1.h"
#include "mask2.h"

namespace
{
    // Tensorflow/Tensorflow Lite constants
    tflite::ErrorReporter* error_reporter = nullptr;
    const tflite::Model* model = nullptr;
    tflite::MicroInterpreter* interpreter = nullptr;
    TfLiteTensor* input = nullptr;
    TfLiteTensor* output = nullptr;
    constexpr int kTensorArenaSize = 100 * 1024;
    static uint8_t tensor_arena[kTensorArenaSize];

    // Constants
    int image_width = 96;
    int image_height = 96;
    int channels = 1;
    int notPersonIndex = 0;
    int personIndex = 1;
    int person_count = 0;
    int not_person_count = 0;
    int other_count = 0;
    int img_list_len = 11;
    
    // Image Arrays
    const uint8_t* person1 = person1_img; const uint8_t* person2 = person2_img;
    const uint8_t* car1 = car1_img;       const uint8_t* car2 = car2_img;
    const uint8_t* car3 = car3_img;       const uint8_t* space1 = space1_img;
    const uint8_t* space2 = space2_img;   const uint8_t* space3 = space3_img;
    const uint8_t* person3 = person3_img; const uint8_t* person4 = person4_img;
    const uint8_t* mask1 = mask1_img;     const uint8_t* person5 = person5_img;
    const uint8_t* person6 = person6_img; const uint8_t* car4 = car4_img;
    const uint8_t* car5 = car5_img;       const uint8_t* space4 = space4_img;
    const uint8_t* person7 = person7_img; const uint8_t* person8 = person8_img;
    const uint8_t* person9 = person9_img; const uint8_t* person10 = person10_img;
    const uint8_t* monkey = monkey_img;   const uint8_t* mask2 = mask2_img;
    
    // Semaphores
    #define STACK_AREA 2024
    #define PRIORITY 1
    K_SEM_DEFINE(loop1_sem, 0, 1);  // This semaphore starts off as not available
    K_SEM_DEFINE(loop2_sem, 0, 1);  // This semaphore starts off as not available
    K_SEM_DEFINE(out_sem, 1, 1);  // This semaphore starts off as available
    K_THREAD_STACK_DEFINE(loop1_stack_area, STACK_AREA);
    static struct k_thread loop1_data;
    K_THREAD_STACK_DEFINE(loop2_stack_area, STACK_AREA);
    static struct k_thread loop2_data;
    K_THREAD_STACK_DEFINE(out_stack_area, STACK_AREA);
    static struct k_thread out_data;

    // Functions
    TfLiteStatus GetImage(
        tflite::ErrorReporter* error_reporter, int width,    int height,
        int channels,                          int8_t* data, const uint8_t* image,
        size_t bytes
    ){
        memcpy(data, image, bytes);
        return kTfLiteOk;
    }
    TfLiteStatus Output(
        tflite::ErrorReporter* error_reporter, uint8_t person_score,
        uint8_t not_person_score
    ){
        TF_LITE_REPORT_ERROR(
            error_reporter,
            "Person score = %d\nNot a person score = %d",
            person_score, not_person_score
        );
        return kTfLiteOk;
    }
    void loop(
            const char* thread_name,   struct k_sem* my_sem, struct k_sem* other_sem,
            const uint8_t* image_list[]
    ){
        for (int i = 0; i < img_list_len; ++i)
        {
            k_sem_take(my_sem, K_FOREVER);
            int64_t t_initial = k_uptime_get();
            printk("%s thread\n", thread_name);
            const uint8_t* image = image_list[i];
            if (kTfLiteOk != GetImage(
                error_reporter,   image_width, image_height, channels,
                input->data.int8, image,       input->bytes
            )){
                TF_LITE_REPORT_ERROR(error_reporter, "GetImage() failed");
            }
            if (kTfLiteOk != interpreter->Invoke())
            {
                TF_LITE_REPORT_ERROR(error_reporter, "Invoke() failed");
            }
            output = interpreter->output(0);
            uint8_t person_score = output->data.uint8[personIndex];
            uint8_t not_person_score = output->data.uint8[notPersonIndex];
            
            if (kTfLiteOk != Output(
                error_reporter, person_score, not_person_score
            )){
                TF_LITE_REPORT_ERROR(error_reporter, "Output() failed");
            }
            if (person_score > not_person_score)
            {
                ++person_count;
                printk("Picture %d is a person\n", i);
            }
            else if (person_score < not_person_score)
            {
                ++not_person_count;
                printk("Picture %d is not a person\n", i);
            }
            else
            {
                ++other_count;
                printk("Could not identify picture %d\n", i);
            }
            int64_t t_final = k_uptime_get();
            printk(
                "Time taken for %s thread: %lld seconds\n\n",
                thread_name,
                (t_final - t_initial) / 1000
            );
            k_sem_give(other_sem);
        }
    }
} // namespace

void setup(void)
{
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;

    model = tflite::GetModel(person_detect);
    if (model->version() != TFLITE_SCHEMA_VERSION)
    {
        error_reporter->Report(
            "Model schema %d is not equal to supported schema version %d",
            model->version(),
            TFLITE_SCHEMA_VERSION
        );
        return;
    }
    static tflite::MicroMutableOpResolver<5> micro_op_resolver;
    micro_op_resolver.AddConv2D();
    micro_op_resolver.AddDepthwiseConv2D();
    micro_op_resolver.AddAveragePool2D();
    micro_op_resolver.AddReshape();
    micro_op_resolver.AddSoftmax();
    
    static tflite::MicroInterpreter static_interpreter(
        model,
        micro_op_resolver,
        tensor_arena,
        kTensorArenaSize,
        error_reporter
    );
    interpreter = &static_interpreter;
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk)
    {
        TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
        return;
    }
    input = interpreter->input(0);
}
void loop1(void* dummy1, void* dummy2, void* dummy3)
{
    ARG_UNUSED(dummy1);
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);

    const uint8_t* img_list[img_list_len] = {
        person1, person2, car1,    car2,    car3, space1,
        space2,  space3,  person3, person4, mask1
    };
    loop(
        __func__, &loop1_sem, &loop2_sem, img_list
    );
}
void loop2(void* dummy1, void* dummy2, void* dummy3)
{
    ARG_UNUSED(dummy1);
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);

    const uint8_t* img_list[img_list_len] = {
        person5, person6, car4,     car5,   space4, person7,
        person8, person9, person10, monkey, mask2
    };
    loop(
        __func__, &loop2_sem, &loop1_sem, img_list
    );
    k_sem_give(&out_sem);
}
void out(void* dummy1, void* dummy2, void* dummy3)
{
    ARG_UNUSED(dummy1);
    ARG_UNUSED(dummy2);
    ARG_UNUSED(dummy3);

    k_sem_take(&out_sem, K_FOREVER);
    int64_t p_initial = k_uptime_get();
    k_sem_give(&loop1_sem);
    k_sem_take(&out_sem, K_FOREVER);
    printk(
        "%s thread\n"
        "Person Count = %d\n"
        "No Person Count = %d\n"
        "Other Count = %d\n\n",
        __func__, person_count, not_person_count, other_count
    );
    int64_t p_final = k_uptime_get();
    printk(
        "Time taken for whole program: %lld seconds\n\n"
        "PROGRAM ENDED\n",
        (p_final - p_initial) / 1000
    );
}
int main()
{
    k_thread_create(
        &out_data, out_stack_area, K_THREAD_STACK_SIZEOF(out_stack_area),
        out, NULL, NULL, NULL, 2, 0, K_FOREVER
    );
    k_thread_start(&out_data);
    printk("STARTING PROGRAM\n\n");
    setup();
    printk("Setup completed\n\n");
    k_thread_create(
        &loop1_data, loop1_stack_area, K_THREAD_STACK_SIZEOF(loop1_stack_area),
        loop1, NULL, NULL, NULL, PRIORITY, 0, K_FOREVER
    );
    k_thread_create(
        &loop2_data, loop2_stack_area, K_THREAD_STACK_SIZEOF(loop2_stack_area),
        loop2, NULL, NULL, NULL, PRIORITY, 0, K_FOREVER
    );
    printk("Starting loop1 thread\n\n");
    k_thread_start(&loop1_data);
    printk("Starting loop2 thread\n\n");
    k_thread_start(&loop2_data);
    return 0;
}
