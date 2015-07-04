// raw2root.cpp
// C. Thornsberry
// June 15th, 2015
// Raw2Root.cpp
// SYNTAX: ./raw2root [filename] <options>

#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <string.h>
#include <sstream>

#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TNamed.h"

bool is_in(const std::vector<int> &nums, int num_){
	for(unsigned int i = 0; i < nums.size(); i++){
		if(nums.at(i) == num_){ return true; }
	}
	return false;
}

std::string get_name(char *input_){
	size_t length = strlen(input_);
	size_t position;
	std::string output = "";
	for(size_t index = length-1; index >= 0; index--){
		if(input_[index] == '.'){
			position = index;
			break;
		}
	}
	for(size_t index = 0; index < position; index++){
		output += input_[index];
	}
	return output;
}

int count_columns(std::string input_, std::vector<std::string> &names, const char delimiter_='\t'){
	names.clear();
	int num_col = 0;
	std::string temp = "";
	for(size_t index = 0; index < input_.size(); index++){
		if(input_[index] == delimiter_){
			if(temp.size() > 0){ // found a new column
				names.push_back(temp);
				num_col++;
				temp = "";
			}
		}
		else{ temp += input_[index]; }
	}
	
	if(temp.size() > 0){ 
		names.push_back(temp);
		num_col++; 
	}
	
	return num_col;
}

void help(char * prog_name_){
	std::cout << "  SYNTAX: " << prog_name_ << " [filename] <options>\n";
	std::cout << "   Available options:\n";
	std::cout << "    --names               | First line of data file contains column names.\n";
	std::cout << "    --header [length]     | Number of lines in the data header.\n";
	std::cout << "    --delimiter [char]    | Supply the data delimiter for line parsing.\n";
	std::cout << "    --skip [num] n1 n2... | Skip num lines from the data file.\n";
}

int main(int argc, char* argv[]){
	if(argc < 2){
		std::cout << " Error: Invalid number of arguments to " << argv[0] << ". Expected 1, received " << argc-1 << ".\n";
		help(argv[0]);
		return 1;
	}

	std::vector<int> skip_lines;
	bool use_column_names = false;
	int index = 2;
	int num_head_lines = 0;
	int last_skip_line = -9999;
	char delimiter = '\t';
	while(index < argc){
		if(strcmp(argv[index], "--names") == 0){
			std::cout << " Using first line of data for column names\n";
			use_column_names = true;
		}
		else if(strcmp(argv[index], "--header") == 0){
			if(index + 1 >= argc){
				std::cout << " Error! Missing required argument to '--header'!\n";
				help(argv[0]);
				return 1;
			}
			num_head_lines = atoi(argv[++index]);
			std::cout << " Using file header of " << num_head_lines << " lines\n";
		}
		else if(strcmp(argv[index], "--delimiter") == 0){
			if(index + 1 >= argc){
				std::cout << " Error! Missing required argument to '--delimiter'!\n";
				help(argv[0]);
				return 1;
			}
			delimiter = (char)atoi(argv[++index]);
			std::cout << " Using column delimiter '" << delimiter << "'\n";
		}
		else if(strcmp(argv[index], "--skip") == 0){
			if(index + 1 >= argc){
				std::cout << " Error! Missing required argument to '--skip'!\n";
				help(argv[0]);
				return 1;
			}
			int num_lines_to_skip = atoi(argv[++index]);
			std::cout << " Skipping " << num_lines_to_skip << " lines in the data file\n";
			if(index + num_lines_to_skip >= argc){
				std::cout << " Error! Missing required argument to '--skip'!\n";
				help(argv[0]);
				return 1;
			}
			int temp_line;
			for(int i = 0; i < num_lines_to_skip; i++){
				temp_line = atoi(argv[++index]);
				skip_lines.push_back(temp_line);
				if(temp_line > last_skip_line){ last_skip_line = temp_line; }
			}
		}
		else{ 
			std::cout << " Error! Unrecognized option '" << argv[index] << "'!\n";
			help(argv[0]);
			return 1;
		}
		index++;
	}

	std::ifstream input_file(argv[1]);
	if(!input_file.good()){
		std::cout << " Error: Failed to open input file " << argv[1] << std::endl;
		return 1;
	}

	std::string fname = get_name(argv[1]);
	TFile *output_file = new TFile((fname+".root").c_str(),"RECREATE");
	if(!output_file->IsOpen()){
		std::cout << " Error: Failed to open output file " << fname << ".root\n";
		input_file.close();
		output_file->Close();
		return 1;
	}
	
	TTree *tree = new TTree("data","Raw2Root Tree");
	int count = 0;
	int num_columns = 0;
	
	output_file->cd();

	std::vector<std::string> names;
	std::vector<std::string> titles;
	std::string first_line;
	if(num_head_lines > 0){
		for(int i = 0; i < num_head_lines; i++){
			std::getline(input_file, first_line);
			if(input_file.eof() || !input_file.good()){
				input_file.close();
				output_file->Close();
				return 1;
			}
			if(is_in(skip_lines, ++count)){ continue; }
			
			size_t index = first_line.find(delimiter);
			if(index != std::string::npos){
				names.push_back(first_line.substr(0, index));
				titles.push_back(first_line.substr(index+1));
			}
			else{ 
				std::stringstream stream;
				if(i < 9){ stream << "line00" << i+1; }
				else if(i < 99){ stream << "line0" << i+1; }
				else{ stream << "line" << i+1; }
				names.push_back(stream.str());
				titles.push_back(first_line);
			}
		}
	}

	/*for(int i = 0; i < names.size(); i++){
		char *cstr1 = new char[names[i].length()+1];
		for(int j = 0; j < names[i].length(); j++){
			cstr1[j] = names[i].at(j);
			//std::cout << names[i][j] << "\t" << cstr1[j] << std::endl;
		}
		cstr1[names[i].length()] = '\0';
		std::cout << names[i].length() << "\t" << strlen(cstr1) << std::endl;
		printf("cstr1: %s\n", cstr1);
		char *cstr2 = new char[titles[i].length()+1];
		for(int j = 0; j < titles[i].length(); j++){
			cstr2[j] = titles[i][j];
		}
		cstr2[titles[i].length()] = '\0';
		delete[] cstr1;
		delete[] cstr2;
	}*/

	// Get the number of columns
	std::getline(input_file, first_line);
	if(input_file.eof() || !input_file.good()){
		input_file.close();
		output_file->Close();
		return 1;
	}

	std::vector<std::string> column_names;
	num_columns = count_columns(first_line, column_names, delimiter);
	std::cout << " Found " << num_columns << " columns of data\n";
	std::string bname;

	float *vars = new float[num_columns];
	for(int i = 0; i < num_columns; i++){
		vars[i] = 0.0;
	}

	if(!use_column_names){	
		input_file.seekg(-first_line.size(), std::ios::cur);
		for(int i = 0; i < num_columns; i++){
			std::stringstream stream;
			if(i < 10){ stream << "Col0" << i; }
			else{ stream << "Col" << i; }
			tree->Branch(stream.str().c_str(),&vars[i]);
		}
	}
	else{ // Extract column names from data
		for(int i = 0; i < num_columns; i++){
			tree->Branch(column_names[i].c_str(),&vars[i]);
		}
	}

	while(true){
		// Get a line of data
		if(count % 10000 == 0 && count != 0){ std::cout << "  Line " << count << " of data file\n"; }
		if(count+1 <= last_skip_line && is_in(skip_lines, ++count)){ continue; }
		
		for(int i = 0; i < num_columns; i++){
			input_file >> vars[i];
		}

		if(input_file.eof() || !input_file.good()){ break; }
		
		// Fill all branches
		tree->Fill();
		count++;
	}

	output_file->cd();
	TNamed *named = new TNamed();
	std::vector<std::string>::iterator iter1, iter2;
	for(iter1 = names.begin(), iter2 = titles.begin(); iter1 != names.end() && iter2 != titles.end(); iter1++, iter2++){
		named->SetNameTitle(iter1->c_str(), iter2->c_str());
		named->Write();
	}
	
	tree->Write();
	
	std::cout << " Found " << count << " entries in " << num_columns << " columns\n";
	std::cout << " Generated file " << fname << ".root\n";

	input_file.close();
	output_file->Close();

	delete[] vars;

	return 0;
}
