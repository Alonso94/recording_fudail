#include<iostream>
#include<stdio.h>
#include "opencv2/opencv.hpp"
#include<fstream>
#include <chrono>
#include<zconf.h>
#include<csignal>
using namespace std;
using namespace cv;
using namespace std::chrono;
//global variables
string directory = "" ; //the directory to store videos
const int FPS = 25;// number of video frames per second
const int video_duration = 60;//video duration 60 sec
long long used_space = 0;
long long max_allowed_space = 3ll * 1024 * 1024 * 1024;//3Gb
int frame_width = 0, frame_height = 0;
bool approximate_file_size = 0; //this option is to reduce the time needed for measuring filesize (about 40 microsecond reduced to 3 microsecond)
                                //by approximating it to the size of the first recorded video
int first_file_size = -1;

int get_file_size(string file_name) {
    if (approximate_file_size && first_file_size != -1)return first_file_size;
    ifstream myfile(file_name, ios::binary);
    int begin = myfile.tellg();
    myfile.seekg(0, ios::end);
    return first_file_size=(int(myfile.tellg()) - begin);
}
string gen_file_name() {//according to that video record will be for the last few hours it is enough to name the videos by the time
    static int day = 60 * 60 * 24;
    unsigned long long x = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
    x %= day;
    return directory+to_string(x / 3600+3) + "_" + to_string((x % 3600) / 60) + "_" + to_string(x % 60) + ".mp4";
}
void free_storage(queue<pair<string, long long> > &in_storage) {
    while (used_space >= max_allowed_space && !in_storage.empty()) {
        string file_name = in_storage.front().first;
        long long sz = in_storage.front().second;
        in_storage.pop();
        used_space -= sz;
        remove(file_name.c_str());
    }
}
void record_frame(Mat& frame, int i,bool exit=0) {
    static VideoWriter video;
    static queue<pair<string, long long> > in_storage;
    static string file_name;
    if (exit) {
        cout << "Saving the file" << endl;
        video.release();
        return;
    }
    if (i == 0) {
        free_storage(in_storage);//delete first files if used storage more than maximum allowed storage
       file_name = gen_file_name();//file name will be the time of starting recording
        cout << "Recording video with name : " << file_name << endl;
        if (!video.open(file_name, VideoWriter::fourcc('m', 'p', '4', 'v'), FPS, Size(frame_width, frame_height)))
            cerr << "unable to start recording file, error in file name";
    }
    video << frame;
    if (i == FPS * video_duration - 1) {
        cout << "Saving the file" << endl;
        video.release();
        long long sz = get_file_size(file_name);
        used_space += sz;
        in_storage.push({ file_name,sz });
    }
}
void signalHandler(int signum) {
    Mat frame;
    record_frame(frame, 0, 1);
    exit(signum);
}
int main()
{
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    double frame_time = 1000.0 / FPS;
    Mat frame;
    VideoCapture cap;
    if (!cap.open(0)) {
        cout << "error opening camera\n";
        return 0;
    }
    cap.read(frame);
    frame_height = frame.rows;
    frame_width = frame.cols;
    while (1) {//start recording new video
        unsigned int start = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count(); //time of start recording video
        for (int i = 0; i < video_duration * FPS; ++i) {
            unsigned int x = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
            x -= start;
            if (x < i * frame_time)
                usleep((i * frame_time - x)*1000);//wait untile next frame time
            cap >> frame;
            //here you are able to implement any process to the frame
            record_frame(frame, i);
        }

    }
}
