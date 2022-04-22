#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_error_reporter.h>
#include <tensorflow/lite/schema/schema_generated.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/micro/micro_mutable_op_resolver.h>
#include <iostream>
#include "person.h"
#include "model.h"
#include "car1.h"
#include "car2.h"
#include "car3.h"
#include "car4.h"
#include "car5.h"
#include "no_person.h"
#include "no_person1.h"
#include "no_person2.h"
#include "no_person3.h"
#include "no_person4.h"
#include "no_person5.h"
#include "person.h"
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
#include "sir_farhan.h"
#include "hamza.h"
#include "monkey.h"
#include "mask1.h"
#include "mask2.h"

namespace
{
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

    // Functions
    TfLiteStatus GetImage(
        tflite::ErrorReporter* error_reporter,
        int width,
        int height,
        int channels,
        int8_t* data,
        const uint8_t* image,
        size_t bytes
    ){
        memcpy(data, image, bytes);
        return kTfLiteOk;
    }
    TfLiteStatus Output(
        tflite::ErrorReporter* error_reporter,
        uint8_t person_score,
        uint8_t not_person_score
    ){
        TF_LITE_REPORT_ERROR(
            error_reporter,
            "Person score = %d\nNot a person score = %d",
            person_score,
            not_person_score
        );
        return kTfLiteOk;
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
void loop()
{
    const uint8_t* person = person_img;
    const uint8_t* person1 = person1_img;
    const uint8_t* person2 = person2_img;
    const uint8_t* no_person = no_person_img;
    const uint8_t* no_person1 = no_person1_img;
    const uint8_t* no_person2 = no_person2_img;
    const uint8_t* car1 = car1_img;
    const uint8_t* car2 = car2_img;
    const uint8_t* car3 = car3_img;
    const uint8_t* space1 = space1_img;
    const uint8_t* space2 = space2_img;
    const uint8_t* space3 = space3_img;
    const uint8_t* person3 = person3_img;
    const uint8_t* person4 = person4_img;
    const uint8_t* person5 = person5_img;
    const uint8_t* person6 = person6_img;
    const uint8_t* car4 = car4_img;
    const uint8_t* car5 = car5_img;
    const uint8_t* no_person3 = no_person3_img;
    const uint8_t* no_person4 = no_person4_img;
    const uint8_t* no_person5 = no_person5_img;
    const uint8_t* space4 = space4_img;
    const uint8_t* person7 = person7_img;
    const uint8_t* person8 = person8_img;
    const uint8_t* person9 = person9_img;
    const uint8_t* person10 = person10_img;
    const uint8_t* hamza = hamza_img;
    const uint8_t* sir_farhan = sir_farhan_img;
    const uint8_t* img_list[28] = {
        person,
        person1,
        person2,
        no_person,
        no_person1,
        no_person2,
        car1,
        car2,
        car3,
        space1,
        space2,
        space3,
        person3,
        person4,
        person5,
        person6,
        car4,
        car5,
        no_person3,
        no_person4,
        no_person5,
        space4,
        person7,
        person8,
        person9,
        person10,
        hamza,
        sir_farhan
    };
    for (int i = 0; i < 28; ++i)
    {
        const uint8_t* image = img_list[i];
        // const uint8_t* image = mask2_img;
        if (kTfLiteOk != GetImage(
            error_reporter,
            image_width,
            image_height,
            channels,
            input->data.int8,
            image,
            input->bytes
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
            error_reporter,
            person_score,
            not_person_score
        )){
            TF_LITE_REPORT_ERROR(error_reporter, "Output() failed");
        }
        if (person_score > not_person_score)
        {
            ++person_count;
        }
        else if (person_score < not_person_score)
        {
            ++not_person_count;
        }
        else
        {
            ++other_count;
            std::cout << "Index: " << i << std::endl;
        }
        
    }
}
int main()
{
    setup();
    loop();
    std::cout << "Person count: " << person_count << std::endl;
    std::cout << "Not Person count: " << not_person_count << std::endl;
    std::cout << "Other count: " << other_count << std::endl;
    return 0;
}

// int main()
// {
//     // Setting up logging
//     tflite::MicroErrorReporter micro_error_reporter;
//     tflite::ErrorReporter* error_reporter = &micro_error_reporter;

//     // Defining tensor area for input, output and intermediate tensors
//     const int kTenorArenaSize = 70 * 1024;
//     uint8_t tensorArena[kTenorArenaSize];

//     // Map model to usable data structure
//     const tflite::Model* model = tflite::GetModel(person_detect);
//     if (model->version() != TFLITE_SCHEMA_VERSION)
//     {
//         error_reporter->Report(
//             "Model provided is schema version %d which is not equal to supported version %d.\n",
//             model->version(),
//             TFLITE_SCHEMA_VERSION
//         );
//     }

//     // Pull in the required operations needed by the graph
//     static tflite::MicroMutableOpResolver<5> micro_op_resolver;
//     micro_op_resolver.AddConv2D();
//     micro_op_resolver.AddDepthwiseConv2D();
//     micro_op_resolver.AddAveragePool2D();
//     micro_op_resolver.AddReshape();
//     micro_op_resolver.AddSoftmax();

//     // Build an interpreter
//     tflite::MicroInterpreter interpreter(
//         model,
//         micro_op_resolver,
//         tensorArena,
//         kTenorArenaSize,
//         error_reporter
//     );
//     interpreter.AllocateTensors();

//     // Feeding the input
//     TfLiteTensor* input = interpreter.input(0);
//     const uint8_t* img = person_image;
//     for (int i = 0; i < input->bytes; i++)
//     {
//         input->data.uint8[i] = img[i];
//     }

//     // Running inference
//     TfLiteStatus invoke_status = interpreter.Invoke();
//     if (invoke_status != kTfLiteOk)
//     {
//         error_reporter->Report("invoke failed\n");
//     }

//     // Getting the output
//     TfLiteTensor* output = interpreter.output(0);
//     uint8_t person_prob = output->data.uint8[1];
//     uint8_t not_person_prob = output->data.uint8[2];

//     // Logging the output
//     error_reporter->Report(
//         "Person Probability = %d\nNote a person Probability = %d",
//         person_prob,
//         not_person_prob
//     );

//     return 0;
// }
