#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>

// File to turn the input csvs into something desmos can use to plot, to help with
// visualization of the test data.
// With desmos' api, it is also

int main(void){
    
    std::cout << "Enter the command for what you wish to do.\n";
    std::cout << "(/d to parse a csv to desmos, /c to parse desmos strings to csv): ";

    std::string s;
    while(std::getline(std::cin,s)){
        if (s == "/d"){
            std::cout << "Enter filename(do not include the test_cases directory): ";
            std::getline(std::cin,s);
            std::ifstream ifs("test_cases/" + s);
            if (!ifs) {
                std::cout << "File "<< "test_cases/" + s <<" not found.\n";
            } else {
                std::string filename = s;
                std::cout << "Prasing file: " << filename << '\n';
                std::vector<std::stringstream> filebufs;

                // getting rid of the first line which is
                // ring_id,vertex_id,x,y
                std::getline(ifs,s);
                int currRing = -1;
                while (std::getline(ifs,s)){
                    int ring_id,vertex_id;
                    double x,y;
                    char c;
                    // Vertex Id is not required.
                    std::stringstream sstr(s);
                    sstr >> ring_id >> c >> vertex_id >> c >> x >> c >> y;
                    if (sstr.fail()) break;  // hit unkown text line

                    // If new ring, push back new entry into vector
                    if (currRing != ring_id){
                        currRing++;
                        filebufs.push_back({});
                        filebufs[ring_id] << std::setprecision(19) << x << ',' << y << '\n';
                    }
                    else {
                        filebufs[ring_id] << x << ',' << y << '\n';
                    }
                }

                std::string fileHeader{"desmos/"};
                fileHeader += filename;
                fileHeader.erase(fileHeader.size() - 4,4);  // removing the .csv tag
                fileHeader += '_';
                for (int i{}; i < filebufs.size(); i++){
                    std::ofstream ofs(fileHeader + std::to_string(i) + ".csv");
                    ofs << filebufs[i].rdbuf();
                }
            }
        }
        else if (s == "/c"){
            std::cout << "Enter the name of the output file (without the test_cases directory): ";
            std::getline(std::cin,s);
            std::ofstream ofs( "test_cases/" + s);
            if (!ofs){
                std::cout << "Invalid filename/directory, could not be opened.\n";
            }
            else {
                std::cout << "Input the amount of loops: ";
                std::getline(std::cin,s);
                int loops = std::stoi(s);
                if (loops < 1) {
                    std::cout << "Input a number equal to or greater than 1 please.\n";
                }
                else {
                    ofs << "ring_id,vertex_id,x,y\n";
                    ofs << std::setprecision(19);
                    for (int i{}; i < loops; i++){
                        std::cout << "Paste the x string from desmos (entire string): ";
                        std::getline(std::cin,s);
                        std::stringstream sstr(s);
                        char c; double d; std::vector<double> x_d,y_d;
                        sstr >> s >> s;
                        while (sstr >> d){
                            x_d.push_back(d);
                            sstr >> c;
                        }

                        std::cout << "Paste the y string from desmos (entire string): ";
                        std::getline(std::cin,s);
                        std::stringstream sstr2(s);
                        sstr2 >> s >> s;
                        while (sstr2 >> d){
                            y_d.push_back(d);
                            sstr2 >> c;
                        }

                        if (x_d.size() != y_d.size()) {
                            std::cout << "Size mismatch detected! Terminating process...\n";
                            break;
                        }

                        for (size_t j{}; j < x_d.size(); j++){
                            ofs << i << ',' << j << ',' << x_d[j] << ',' << y_d[j] << '\n';
                        }
                    }
                }
                std::cout << "Finished writing file.\n";
            }
        }
        else {
            std::cout << "Unkown command\n";
        }

        std::cout << "\n\n";
        std::cout << "Enter the command for what you wish to do.\n";
        std::cout << "(/d to parse a csv to desmos, /c to parse desmos strings to csv): ";
    }

    return 0;
}

/* For extracting desmos data, copy paste this javascript:

state = Calc.getState()

for (let i = 0; i < state.expressions.list.length; i++) {
  if (state.expressions.list[i].type == "table") {
    for (let j = 0; j < state.expressions.list[i].columns.length; j++) {
      console.log(state.expressions.list[i].columns[j].latex + " = " + state.expressions.list[i].columns[j].values.toString())
    }
  }
}

*/
// Javascript code grabbed from a random reddit post