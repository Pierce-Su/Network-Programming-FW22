#include <aws/s3/S3Client.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <iostream>

int main() {
  Aws::SDKOptions options;
  Aws::InitAPI(options);
  {

    // S3 Bucket 名稱
    const Aws::String bucketName = "genghisswan";

    // 檔案名稱與內容
    const Aws::String objectName = "server_status02.txt";
    const std::string objectContent = "0\n";


    // 建立 S3 Client
    Aws::Client::ClientConfiguration config;
    config.verifySSL = false;

    Aws::S3::S3Client s3Client;


    // 建立上傳 Object 至 S3 的請求
    Aws::S3::Model::PutObjectRequest putObjectRequest;
    putObjectRequest.SetBucket(bucketName);
    putObjectRequest.SetKey(objectName);

    // 建立資料輸入串流，可為任何串流，例如 fstream、stringstream 等
    const std::shared_ptr<Aws::IOStream> inputStream = Aws::MakeShared<Aws::StringStream>("");
    *inputStream << objectContent.c_str();

    // 設定上傳至 S3 的資料輸入串流
    putObjectRequest.SetBody(inputStream);

    // 將 Object 上傳至 S3
    Aws::S3::Model::PutObjectOutcome putObjectOutcome = s3Client.PutObject(putObjectRequest);

    if(putObjectOutcome.IsSuccess()) {
      std::cout << "上傳成功" << std::endl;
    } else {
      std::cout << "上傳失敗：" << putObjectOutcome.GetError().GetExceptionName()
                << " " << putObjectOutcome.GetError().GetMessage() << std::endl;
    }

    // 建立從 S3 下載 Object 的請求
    Aws::S3::Model::GetObjectRequest getObjectRequest;
    getObjectRequest.SetBucket(bucketName);
    getObjectRequest.SetKey(objectName);

    // 從 S3 下載 Object
    Aws::S3::Model::GetObjectOutcome getObjectOutcome = s3Client.GetObject(getObjectRequest);

    if(getObjectOutcome.IsSuccess()) {
      std::cout << "下載成功，資料內容：" << std::endl;
      std::cout << getObjectOutcome.GetResult().GetBody().rdbuf() << std::endl;
    } else {
      std::cout << "下載失敗：" << getObjectOutcome.GetError().GetExceptionName() << " "
                << getObjectOutcome.GetError().GetMessage() << std::endl;
    }
  }
  Aws::ShutdownAPI(options);
  return 0;
}