sudo apt-get install -y build-essential
sudo apt-get install -y cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
sudo apt-get install -y python-dev python-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libjasper-dev libdc1394-22-dev

mkdir opencv_install
# shellcheck disable=SC2164
cd opencv_install
git clone https://github.com/opencv/opencv.git
git clone https://github.com/opencv/opencv_contrib.git

cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/usr/local ..

make -j4

cd ..