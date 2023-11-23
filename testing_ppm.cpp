#include "ppm_utils.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <omp.h>
#include <random>
#include <string>
#include <vector>
using namespace std;
namespace fs = filesystem;

string dirpath = "/home/owaisk4/Win_backup/FAST NU assignments/Parallel and Distributed Computing/Project/Image Processing/Code/Images/";
vector<string> files;
double start_time, end_time, elapsed, total;     // For calculating local benchmarks
double global_start, global_end, global_elapsed; // For calculating global benchmarks
double IO_start, IO_end, IO_time = 0;            // Extra time taken by IO

int main() {
    for (const auto &entry : fs::directory_iterator(dirpath)) {
        if (entry.is_directory())
            continue;
        string file = entry.path().string();
        if (file.find("ppm", 0) != file.npos)
            files.push_back(file);
    }
    cout << "Total no. of files: " << files.size() << "\n";
    int threads = 8;
    cout << "Enter no. of threads to use for conversion: ";
    cin >> threads;
    threads = max(1, min(8, threads));

    global_start = omp_get_wtime();
    for (string filename : files) {
        if (filename.find("ppm", 0) == filename.npos)
            continue;
        fstream infile(filename, ios::in);
        if (infile.fail()) {
            cout << "Input file doesn't exist. Exiting.\n";
            return 0;
        }
        string enhanced_image_filename = filename.substr(0, filename.size() - 4) + "_enhanced_local.ppm";
        fstream enhancedoutfile(enhanced_image_filename, ios::out);
        string shortened_filename = filename.substr(filename.rfind("/") + 1, filename.size() - filename.rfind("/"));

        IO_start = omp_get_wtime(); // For IO time
        PPMObject ppm;
        infile >> ppm;
        cout << "Completed reading from file: " << shortened_filename << "\n";
        IO_end = omp_get_wtime();                // For IO time
        IO_time = IO_time + (IO_end - IO_start); // For IO time

        start_time = omp_get_wtime();
        vector<HSV> hsv = RGB_to_HSV(ppm, threads);
        vector<HSV> blurred_hsv = BlurImage(ppm, hsv, threads);
        vector<HSV> image_mask = GenerateImageMask(hsv, blurred_hsv, threads);
        vector<HSV> sharpened = SharpenImage(hsv, image_mask, threads);
        HSV_to_RGB(ppm, sharpened, threads);

        end_time = omp_get_wtime();
        elapsed = end_time - start_time;
        cout << "Time taken = " << elapsed << " seconds.\n";

        IO_start = omp_get_wtime(); // For IO time
#pragma omp parallel sections
        {
#pragma omp section
            {
                enhancedoutfile << ppm;
            }
        }
        cout << "Completed writing to file.\n";
        IO_end = omp_get_wtime();                // For IO time
        IO_time = IO_time + (IO_end - IO_start); // For IO time

        infile.close();
        enhancedoutfile.close();
    }

    global_end = omp_get_wtime();
    global_elapsed = global_end - global_start - IO_time;
    cout << "Total time taken = " << global_elapsed << " seconds.\n";

    fstream csv(dirpath + "times.csv", ios::app);
    csv << threads << "," << global_elapsed << "\n";
    csv.close();

    return 0;
}

// fstream infile(dirpath + filename + ".ppm", ios::in);
// fstream gsoutfile(dirpath + filename + "_grayscale.ppm", ios::out);
// fstream redoutfile(dirpath + filename + "_rgb_red.ppm", ios::out);
// fstream greenoutfile(dirpath + filename + "_rgb_green.ppm", ios::out);
// fstream blueoutfile(dirpath + filename + "_rgb_blue.ppm", ios::out);
// fstream bluroutfile(dirpath + filename + "_blurred.ppm", ios::out);

// PPMObject grayscale = RGB_to_Grayscale(ppm, threads);
// PPMObject red_ppm = RGB_Extract_Red(ppm, threads);
// PPMObject green_ppm = RGB_Extract_Green(ppm, threads);
// PPMObject blue_ppm = RGB_Extract_Blue(ppm, threads);
// PPMObject blur_ppm(ppm);
// HSV_to_RGB(blur_ppm, blurred_hsv, threads);

// #pragma omp section
//         {
//             gsoutfile << grayscale;
//         }
// #pragma omp section
//         {
//             redoutfile << red_ppm;
//         }
// #pragma omp section
//         {
//             greenoutfile << green_ppm;
//         }
// #pragma omp section
//         {
//             blueoutfile << blue_ppm;
//         }
// #pragma omp section
//         {
//             bluroutfile << blur_ppm;
//         }

// bluroutfile.close();
// gsoutfile.close();
// redoutfile.close();
// greenoutfile.close();
// blueoutfile.close();
// bluroutfile.close();