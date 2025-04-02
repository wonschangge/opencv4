#include <opencv2/opencv.hpp>
#include <opencv2/core/types.hpp>

#include <string>
#include <vector>

const std::vector<std::pair<int, int>> backend_target_pairs = {
    {cv::dnn::DNN_BACKEND_OPENCV, cv::dnn::DNN_TARGET_CPU},
    {cv::dnn::DNN_BACKEND_CUDA,   cv::dnn::DNN_TARGET_CUDA},
    {cv::dnn::DNN_BACKEND_CUDA,   cv::dnn::DNN_TARGET_CUDA_FP16},
    {cv::dnn::DNN_BACKEND_TIMVX,  cv::dnn::DNN_TARGET_NPU},
    {cv::dnn::DNN_BACKEND_CANN,   cv::dnn::DNN_TARGET_NPU}
};

class SFace
{
  public:
    SFace(const std::string& model_path,
          const int backend_id,
          const int target_id,
          const int distance_type)
        : _distance_type(static_cast<cv::FaceRecognizerSF::DisType>(distance_type))
    {
        _recognizer = cv::FaceRecognizerSF::create(model_path, "", backend_id, target_id);
    }

    cv::Mat extractFeatures(const cv::Mat& orig_image, const cv::Mat& face_image)
    {
        // Align and crop detected face from original image
        cv::Mat target_aligned;
        _recognizer->alignCrop(orig_image, face_image, target_aligned);
        // Extract features from cropped detected face
        cv::Mat target_features;
        _recognizer->feature(target_aligned, target_features);
        return target_features.clone();
    }

    std::pair<double, bool> matchFeatures(const cv::Mat& target_features, const cv::Mat& query_features)
    {
        const double score = _recognizer->match(target_features, query_features, _distance_type);
        if (_distance_type == cv::FaceRecognizerSF::DisType::FR_COSINE)
        {
            return {score, score >= _threshold_cosine};
        }
        return {score, score <= _threshold_norml2};
    }

  private:
    cv::Ptr<cv::FaceRecognizerSF> _recognizer;
    cv::FaceRecognizerSF::DisType _distance_type;
    double _threshold_cosine = 0.363;
    double _threshold_norml2 = 1.128;
};

int main()
{
    const int backend = 0;
    const int backend_id = backend_target_pairs.at(backend).first;
    const int target_id = backend_target_pairs.at(backend).second;
    const std::string model_path = "assets/face_recognition_sface_2021dec_int8.onnx";
    const int distance_type = 0;

    auto face_recognizer = SFace(model_path, backend_id, target_id, distance_type);

    // 从本地文件读取 target_features
    cv::FileStorage fs_read("assets/target_features.xml", cv::FileStorage::READ);
    cv::Mat target_features;
    fs_read["target_features"] >> target_features;
    fs_read.release();
    
    // 从本地文件读取 query_features
    cv::FileStorage fs_read_2("assets/query_features.xml", cv::FileStorage::READ);
    cv::Mat query_features;
    fs_read_2["query_features"] >> query_features;
    fs_read_2.release();

    const auto match = face_recognizer.matchFeatures(target_features, query_features);
    std::cout << cv::format("match=%.4f\n", match.first);

    return 0;
}
