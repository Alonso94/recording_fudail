#include<iostream>
#include "opencv2/opencv.hpp"
#include<fstream>
#include <chrono>
#include <zconf.h>

using namespace std;
using namespace cv;
using namespace std::chrono;
string directory = "../" ;
const int FPS = 25;
const int video_duration = 60;//60 sec
double frame_time = 1000.0 / FPS;
long long used_space = 0;
long long max_allowed_space = 3ll * 1024 * 1024 * 1024;//3Gb
Mat frame;
VideoCapture cap;
VideoWriter video;
int frame_width = 0, frame_height = 0;
int get_file_size(string file_name) {
    ifstream myfile(file_name, ios::binary);
    int begin = myfile.tellg();
    myfile.seekg(0, ios::end);
    return int(myfile.tellg()) - begin;
}

inline void start_recording(string file_name) {
    if(!video.open(file_name, VideoWriter::fourcc('m', 'p', '4', 'v'), FPS, Size(frame_width, frame_height)))
        cout<<"File created";
}

queue<pair<string,long long> > in_storage;
inline void end_recording(string file_name) {
    video.release();
    long long sz = get_file_size(file_name);
    used_space += sz;
    in_storage.push({ file_name,sz });
}
int day = 60 * 60 * 24;
string gen_file_name() {//according to that video record will be for the last few hours it is enough to name the videos by the time 
    unsigned long long x = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
    x %= day;
    return directory+to_string(x / 3600+3) + "_" + to_string((x % 3600) / 60) + "_" + to_string(x % 60) + ".mp4";
}
void free_storage() {
    while (used_space >= max_allowed_space && !in_storage.empty()) {
        string file_name = in_storage.front().first;
        long long sz = in_storage.front().second;
        in_storage.pop();
        used_space -= sz;
        remove(file_name.c_str());
    }
}
int main()
{
    if (!cap.open(0)) {
        cout << "error opening camera\n";
        return 0;
    }
    cap.read(frame);
    frame_height = frame.rows;
    frame_width = frame.cols;
    string file_name;
    while (1) {
        //start recording new video
        free_storage();
        file_name = gen_file_name();
        cout << file_name << endl;
        start_recording(file_name);
        unsigned int start = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        for (int i = 0; i < video_duration * FPS; ++i) {
            unsigned int x = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
            x -= start;
            if (x < i * frame_time)
                sleep((i * frame_time - x));
            cap >> frame;
            video << frame;
        }
        end_recording(file_name);
        cout << used_space / (1024 * 1024) << endl;
    }
	
}