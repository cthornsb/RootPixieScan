#include <iostream>
#include <string.h>
#include <ctime>

#include "rcbuild.hpp"

#define VERSION "1.0.0"

bool SplitStr(const std::string &input_, std::string &out1, std::string &out2){
	out1 = "";
	out2 = "";
	bool left_side = true;
	for(size_t i = 0; i < input_.size(); i++){
		if(input_[i] == '\t'){ 
			left_side = false;
			continue;
		}
		
		if(left_side){ out1 += input_[i]; }
		else{ out2 += input_[i]; }
	}
	return true;
}

DataType::DataType(const std::string &type_, const std::string &name_, const std::string &description_){
	SetType(type_);
	name = name_; 
	descrip = description_;

	if(name.find("_wave") != std::string::npos){ trace_value = true; }
	else{ trace_value = false; }
	
	if(name.find("_mult") != std::string::npos){ mult_value = true; }
	else{ mult_value = false; }
	
	is_vector = false;
}

DataType::DataType(const std::string &entry_, const char &delimiter){
	std::string v1 = "";
	name = "";
	descrip = "";
	
	unsigned int count = 0;
	for(size_t i = 0; i < entry_.size(); i++){
		if(entry_[i] == delimiter){
			if(++count == 4){ break; }
			continue;
		}
		else if(count == 0){ v1 += entry_[i]; }
		else if(count == 1){ name += entry_[i]; }
		else if(count == 2){ descrip += entry_[i]; }
		else{ break; }
	}

	if(name.find("_wave") != std::string::npos){ trace_value = true; }
	else{ trace_value = false; }
	
	if(name.find("_mult") != std::string::npos){ mult_value = true; }
	else{ mult_value = false; }
	
	SetType(v1);
}

void DataType::SetType(const std::string &input_){
	is_vector = false;
	
	type = "";
	decl = "";
	
	size_t index = input_.find("vector:");
	if(index != std::string::npos){
		is_vector = true;
		type = input_.substr(index + 7);
	}
	else{ type = input_; }
	
	index = type.find("u_");
	if(index != std::string::npos){ 
		type = "unsigned " + type.substr(index + 2);
	}
	
	if(is_vector){ decl = "std::vector<" + type + ">"; }
	else{ decl = type; }
}

void DataType::Print(){
	std::cout <<type << "\t" << decl << "\t" << name << "\t" << descrip << std::endl;
}

StructureEntry::StructureEntry(){
	name = "Temp"; shorter = "Temp" ; longer = "Temp";
}

void StructureEntry::push_back(const std::string &entry){
	DataType *data = new DataType(entry);
	
	if(!data->trace_value){ structure_types.push_back(data); }
	else{ waveform_types.push_back(data); }
}

void StructureEntry::WriteHeader(std::ofstream *file_){
	if(!file_ || !file_->good() || file_->eof()){ return; }

	std::string suffix[2] = {"Structure", "Waveform"};
	for(unsigned short i = 0; i < 2; i++){
		if(i == 0 && structure_types.size() == 0){ continue; }
		else if(i == 1 && waveform_types.size() == 0){ continue; }
	
		// Write the class description
		(*file_) << "\n/** " << name << suffix[i] << "\n";
		(*file_) << " * \\brief " << shorter << "\n";
		(*file_) << " *\n";
		(*file_) << " * " << longer << "\n";
		(*file_) << " */\n";

		(*file_) << "class " << name << suffix[i] << " : public " << suffix[i] << " {\n";
		(*file_) << "  public:\n";

		if(i == 0){ // Write the structure variables
			for(std::vector<DataType*>::iterator iter = structure_types.begin(); iter != structure_types.end(); iter++){
				(*file_) << "\t" << (*iter)->decl << " " << (*iter)->name << "; /// " << (*iter)->descrip << std::endl;
			}
		}
		else{
			for(std::vector<DataType*>::iterator iter = waveform_types.begin(); iter != waveform_types.end(); iter++){
				(*file_) << "\t" << (*iter)->decl << " " << (*iter)->name << "; /// " << (*iter)->descrip << std::endl;
			}
		}

		(*file_) << "\n\t/// Default constructor\n";
		(*file_) << "\t" << name << suffix[i] << "();\n\n";

		(*file_) << "\t/// Copy constructor\n";
		(*file_) << "\t" << name << suffix[i] << "(const " << name << suffix[i] << " &other_);\n\n";

		(*file_) << "\t/// Destructor. Does nothing\n";
		(*file_) << "\t~" << name << suffix[i] << "(){}\n\n";

		(*file_) << "\t/// Default constructor\n";
		(*file_) << "\tvoid Append(";
		if(i == 0){ // Write the structure variables
			(*file_) << "const " << structure_types.front()->type << " &" << structure_types.front()->name << "_";
			for(std::vector<DataType*>::iterator iter = structure_types.begin()+1; iter != structure_types.end(); iter++){
				if((*iter)->trace_value || (*iter)->mult_value){ continue; }
				if(!(*iter)->mult_value){ (*file_) << ", const " << (*iter)->type << " &" << (*iter)->name << "_"; }
			}
		}
		else{ // Write the waveform variables
			(*file_) << "const " << waveform_types.front()->decl << " &" << waveform_types.front()->name << "_";
			for(std::vector<DataType*>::iterator iter = waveform_types.begin()+1; iter != waveform_types.end(); iter++){
				if(!(*iter)->trace_value || (*iter)->mult_value){ continue; }
				if(!(*iter)->mult_value){ (*file_) << ", const " << (*iter)->decl << " &" << (*iter)->name << "_"; }
			}
		}
		(*file_) << ");\n\n";

		(*file_) << "\t/// Zero the data " << suffix[i] << "\n";
		(*file_) << "\tvoid Zero();\n\n";

		(*file_) << "\t/// Assignment operator\n";
		(*file_) << "\t" << name << suffix[i] << " &operator = (const " << name << suffix[i] << " &other_);\n\n";
	
		(*file_) << "\t/// Set all values to that of other_\n";
		(*file_) << "\t" << name << suffix[i] << " &Set(const " << name << suffix[i] << " &other_);\n\n";
	
		(*file_) << "\t/// Set all values to that of other_\n";
		(*file_) << "\t" << name << suffix[i] << " &Set(" << name << suffix[i] << " *other_);\n\n";

		(*file_) << "\tClassDef(" << name << suffix[i] << ", 1); // " << name << "\n";
		(*file_) << "};\n";
	}
}

void StructureEntry::WriteSource(std::ofstream *file_){
	if(!file_ || !file_->good() || file_->eof()){ return; }

	std::string suffix[2] = {"Structure", "Waveform"};
	for(unsigned short i = 0; i < 2; i++){
		if(i == 0 && structure_types.size() == 0){ continue; }
		else if(i == 1 && waveform_types.size() == 0){ continue; }

		(*file_) << "\n///////////////////////////////////////////////////////////\n";
		(*file_) << "// " << name << suffix[i] << "\n";
		(*file_) << "///////////////////////////////////////////////////////////\n";
	
		// Default constructor
		(*file_) << name << suffix[i] << "::" << name << suffix[i] << "() : " << suffix[i] << "(\"" << name << suffix[i] << "\") {\n";
		if(i == 0){
			for(std::vector<DataType*>::iterator iter = structure_types.begin(); iter != structure_types.end(); iter++){
				if(!(*iter)->is_vector){ (*file_) << "\t" << (*iter)->name << " = 0;\n"; }
			}
		}
		else{
			for(std::vector<DataType*>::iterator iter = waveform_types.begin(); iter != waveform_types.end(); iter++){
				if(!(*iter)->is_vector){ (*file_) << "\t" << (*iter)->name << " = 0;\n"; }
			}
		}
		(*file_) << "}\n\n";
	
		// Copy constructor
		(*file_) << name << suffix[i] << "::" << name << suffix[i] << "(const " << name << suffix[i] << " &other_) : " << suffix[i] << "(\"" << name << suffix[i] << "\") {\n";
		(*file_) << "\tSet(other_);\n";
		(*file_) << "}\n\n";
	
		// Append
		(*file_) << "void " << name << suffix[i] << "::Append(";
		if(i == 0 && !structure_types.front()->mult_value){ // Write the structure variables
			(*file_) << "const " << structure_types.front()->type << " &" << structure_types.front()->name << "_";
			for(std::vector<DataType*>::iterator iter = structure_types.begin()+1; iter != structure_types.end(); iter++){
				if((*iter)->trace_value || (*iter)->mult_value){ continue; }
				if(!(*iter)->mult_value){ (*file_) << ", const " << (*iter)->type << " &" << (*iter)->name << "_"; }
			}
			(*file_) << "){\n";
			for(std::vector<DataType*>::iterator iter = structure_types.begin(); iter != structure_types.end(); iter++){
				if((*iter)->trace_value || (*iter)->mult_value){ continue; }
				if((*iter)->is_vector){ (*file_) << "\t" << (*iter)->name << ".push_back(" << (*iter)->name << "_);\n"; }
				else if(!(*iter)->mult_value){ (*file_) << "\t" << (*iter)->name << " = " << (*iter)->name << "_;\n"; }
				else{ (*file_) << "\t" << (*iter)->name << "++;\n"; }
			}
		}
		else if(!structure_types.front()->mult_value){ // Write the waveform variables
			(*file_) << "const " << waveform_types.front()->decl << " &" << waveform_types.front()->name << "_";
			for(std::vector<DataType*>::iterator iter = waveform_types.begin()+1; iter != waveform_types.end(); iter++){
				if(!(*iter)->trace_value || (*iter)->mult_value){ continue; }
				if(!(*iter)->mult_value){ (*file_) << ", const " << (*iter)->decl << " &" << (*iter)->name << "_"; }
			}
			(*file_) << "){\n";
			for(std::vector<DataType*>::iterator iter = waveform_types.begin(); iter != waveform_types.end(); iter++){
				if(!(*iter)->trace_value || (*iter)->mult_value){ continue; }
				if(!(*iter)->mult_value){ (*file_) << "\t" << (*iter)->name << " = " << (*iter)->name << "_;\n"; }
				else{ (*file_) << "\t" << (*iter)->name << "++;\n"; }
			}
		}
		(*file_) << "}\n\n";
	
		// Zero
		(*file_) << "void " << name << suffix[i] << "::Zero(){\n";
		if(i == 0){
			for(std::vector<DataType*>::iterator iter = structure_types.begin(); iter != structure_types.end(); iter++){
				if((*iter)->mult_value){ 
					(*file_) << "\tif(" << (*iter)->name << " == 0){ return; } // " << suffix[i] << " is already empty\n";
					continue;
				}
			}
			for(std::vector<DataType*>::iterator iter = structure_types.begin(); iter != structure_types.end(); iter++){
				if((*iter)->is_vector){ (*file_) << "\t" << (*iter)->name << ".clear();\n"; }
				else{ (*file_) << "\t" << (*iter)->name << " = 0;\n"; }
			}
		}
		else{
			for(std::vector<DataType*>::iterator iter = waveform_types.begin(); iter != waveform_types.end(); iter++){
				if((*iter)->mult_value){ 
					(*file_) << "\tif(" << (*iter)->name << " == 0){ return; } // " << suffix[i] << " is already empty\n";
					continue;
				}
			}
			for(std::vector<DataType*>::iterator iter = waveform_types.begin(); iter != waveform_types.end(); iter++){
				if((*iter)->is_vector){ (*file_) << "\t" << (*iter)->name << ".clear();\n"; }
				else{ (*file_) << "\t" << (*iter)->name << " = 0;\n"; }
			}			
		}
		(*file_) << "}\n\n";
	
		// Assignment operator
		(*file_) << name << suffix[i] << " &" << name << suffix[i] << "::operator = (const " << name << suffix[i] << " &other_){\n";
		(*file_) << "\treturn Set(other_);\n";
		(*file_) << "}\n\n";
	
		// Set (const reference)
		(*file_) << name << suffix[i] << " &" << name << suffix[i] << "::Set(const " << name << suffix[i] << " &other_){\n";
		if(i == 0){
			for(std::vector<DataType*>::iterator iter = structure_types.begin(); iter != structure_types.end(); iter++){
				(*file_) << "\t" << (*iter)->name << " = other_." << (*iter)->name << ";\n";
			}
		}
		else{
			for(std::vector<DataType*>::iterator iter = waveform_types.begin(); iter != waveform_types.end(); iter++){
				(*file_) << "\t" << (*iter)->name << " = other_." << (*iter)->name << ";\n";
			}
		}
		(*file_) << "\treturn *this;\n";
		(*file_) << "}\n\n";
	
		// Set (pointer)
		(*file_) << name << suffix[i] << " &" << name << suffix[i] << "::Set(" << name << suffix[i] << " *other_){\n";
		if(i == 0){
			for(std::vector<DataType*>::iterator iter = structure_types.begin(); iter != structure_types.end(); iter++){
				(*file_) << "\t" << (*iter)->name << " = other_->" << (*iter)->name << ";\n";
			}
		}
		else{
			for(std::vector<DataType*>::iterator iter = waveform_types.begin(); iter != waveform_types.end(); iter++){
				(*file_) << "\t" << (*iter)->name << " = other_->" << (*iter)->name << ";\n";
			}
		}
		(*file_) << "\treturn *this;\n";
		(*file_) << "}\n";
	}
}

void StructureEntry::WriteLinkDef(std::ofstream *file_){
	if(structure_types.size() > 0){
		(*file_) << "#pragma link C++ class " << name << "Structure+;\n";
	}
	if(waveform_types.size() > 0){
		(*file_) << "#pragma link C++ class " << name << "Waveform+;\n";
	}
}

void StructureEntry::Clear(){
	for(std::vector<DataType*>::iterator iter = structure_types.begin(); iter != structure_types.end(); iter++){
		delete (*iter);
	}
	structure_types.clear();
	for(std::vector<DataType*>::iterator iter = waveform_types.begin(); iter != waveform_types.end(); iter++){
		delete (*iter);
	}
	waveform_types.clear();
}

bool StructureFile::Open(){
	if(init){ return false; }

	hppfile.open(hpp_filename.c_str());
	if(!hppfile.good()){ return false; }

	cppfile.open(cpp_filename.c_str());
	if(!cppfile.good()){ 
		hppfile.close();
		return false; 
	}
	
	linkfile.open(link_filename.c_str());
	if(!linkfile.good()){ 
		hppfile.close();
		cppfile.close();
		return false; 
	}

	time_t rawtime;
	struct tm * timeinfo;

	time(&rawtime);
	timeinfo = localtime (&rawtime);

	hppfile << "/** \\file Structures.h\n";
	hppfile << " * \\brief Data structures for root output\n";
	hppfile << " *\n";
	hppfile << " * Special data types for Root output. Each individual processor which is\n";
	hppfile << " * is used in the scan code should have its own Structure class. These classes\n";
	hppfile << " * should contain simple C++ data types or vectors of simple C++ data types.\n";
	hppfile << " * Vectors should be used for processors which are likely to have multiplicities\n";
	hppfile << " * greater than one.\n\n";
	hppfile << " * File automatically generated by\n";
	hppfile << " *  RootClassBuilder (v. " << VERSION << ") on " << asctime(timeinfo);
	hppfile << " *\n";
	hppfile << " * \\author C. Thornsbery\n";
	hppfile << " * \\date Sept. 25, 2014\n";
	hppfile << " */\n\n";
	hppfile << "#ifndef ROOTDATAStructure_H\n";
	hppfile << "#define ROOTDATAStructure_H\n\n";
	hppfile << "#include \"TObject.h\"\n\n";
	hppfile << "#include <vector>\n\n";
	hppfile << "class Structure : public TObject {\n";
	hppfile << "  protected:\n";
	hppfile << "	std::string name; //! Structure name\n\n";
	hppfile << "  public:\n";
	hppfile << "	Structure(const std::string &name_=\"\"){ name = name_; }\n\n";
	hppfile << "	virtual ~Structure(){}\n\n";
	hppfile << "	virtual void Zero(){}\n\n";
	hppfile << "	virtual Structure &operator = (const Structure &other_){ return Set(other_); }\n\n";
	hppfile << "	virtual Structure &Set(const Structure &other_){ return *this; }\n\n";
	hppfile << "	virtual Structure &Set(Structure *other_){ return *this; }\n";
	hppfile << "};\n\n";
	hppfile << "class Waveform : public TObject {\n";
	hppfile << "  protected:\n";
	hppfile << "	std::string name; //! Waveform name\n\n";
	hppfile << "  public:\n";
	hppfile << "	Waveform(const std::string &name_=\"\"){ name = name_; }\n\n";
	hppfile << "	virtual ~Waveform(){}\n\n";
	hppfile << "	virtual void Zero(){}\n\n";
	hppfile << "	virtual Waveform &operator = (const Waveform &other_){ return Set(other_); }\n\n";
	hppfile << "	virtual Waveform &Set(const Waveform &other_){ return *this; }\n\n";
	hppfile << "	virtual Waveform &Set(Waveform *other_){ return *this; }\n";
	hppfile << "};\n";
	
	cppfile << "#include \"Structures.h\"\n";

	linkfile << "#ifdef __CINT__\n\n";
	linkfile << "#include <vector>\n\n";
	linkfile << "#pragma link off all globals;\n";
	linkfile << "#pragma link off all classes;\n";
	linkfile << "#pragma link off all functions;\n\n";
	
	init = true;
	
	return true;
}

bool StructureFile::Open(const std::string &prefix_){
	if(prefix_[prefix_.size()-1] != '/'){
		hpp_filename = prefix_ + "/include/Structures.h";
		cpp_filename = prefix_ + "/src/Structures.h";
		link_filename = prefix_ + "/dict/LinkDef.h";
	}
	else{
		hpp_filename = prefix_ + "include/Structures.h";
		cpp_filename = prefix_ + "src/Structures.h";
		link_filename = prefix_ + "dict/LinkDef.h";
	}
	return Open();
}

bool StructureFile::Open(const std::string &hpp_fname_, const std::string &cpp_fname_, const std::string &link_fname_){
	hpp_filename = hpp_fname_;
	cpp_filename = cpp_fname_;
	link_filename = link_fname_;
	return Open();
}

bool StructureFile::Process(const std::string &filename_){
	if(!init){ return false; }

	std::ifstream entry_file(filename_.c_str());
	if(!entry_file.good()){ return false; }

	StructureEntry *entry = new StructureEntry();

	std::string class_name;
	std::string brief_descrip;
	std::string long_descrip;

	bool running = true;
	std::string line, name, arg;
	while(running){
		getline(entry_file, line);
		if(entry_file.eof()){ break; }
		if(line == "" || line[0] == '#'){ continue; }
		
		SplitStr(line, name, arg);
		
		if(name == "BEGIN_CLASS"){
			class_name = arg;
		}
		else if(name == "SHORT"){
			brief_descrip = arg;
		}
		else if(name == "LONG"){
			long_descrip = arg;
		}
		else if(name == "BEGIN_TYPES"){
			while(true){
				getline(entry_file, line);
				if(entry_file.eof()){ 
					running = false;
					break; 
				}
				if(line == "" || line[0] == '#'){ continue; }
	
				if(line == "END_TYPES"){ break; }
	
				if(running){
					entry->push_back(line);
				}
			}
		}
		else if(name == "END_CLASS"){
			entry->SetName(class_name);
			entry->SetBrief(brief_descrip);
			entry->SetDescription(long_descrip);
		
			entry->WriteHeader(&hppfile);
			entry->WriteSource(&cppfile);
			entry->WriteLinkDef(&linkfile);
			entry->Clear();
		}
		else{ continue; }
	}
	
	delete entry;
	
	entry_file.close();
	
	return true;
}

void StructureFile::Close(){
	if(!init){ return; }

	hppfile << "\n#endif\n";
	linkfile << "\n#endif\n";
	
	hppfile.close();
	cppfile.close();
	linkfile.close();
}

void help(char * prog_name_){
	std::cout << "  SYNTAX: " << prog_name_ << " [input] <options>\n";
	std::cout << "   Available options:\n";
	std::cout << "    --src-dir [dir]  | Specify the source directory (default=./src/).\n";
	std::cout << "    --inc-dir [dir]  | Specify the include directory (default=./include/).\n";
	std::cout << "    --dict-dir [dir] | Specify the dictionary directory (default=./dict/).\n";
}

int main(int argc, char *argv[]){
	if(argc < 2){
		std::cout << " Error: Invalid number of arguments to " << argv[0] << ". Expected 1, received " << argc-1 << ".\n";
		help(argv[0]);
		return 1;
	}

	std::string src_dir = "./src/";
	std::string inc_dir = "./include/";
	std::string dict_dir = "./dict/";

	int index = 2;
	while(index < argc){
		if(strcmp(argv[index], "--src-dir") == 0){
			if(index + 1 >= argc){
				std::cout << " Error! Missing required argument to '--src-dir'!\n";
				help(argv[0]);
				return 1;
			}
			src_dir = std::string(argv[++index]);
		}
		else if(strcmp(argv[index], "--inc-dir") == 0){
			if(index + 1 >= argc){
				std::cout << " Error! Missing required argument to '--inc-dir'!\n";
				help(argv[0]);
				return 1;
			}
			inc_dir = std::string(argv[++index]);
		}
		else if(strcmp(argv[index], "--dict-dir") == 0){
			if(index + 1 >= argc){
				std::cout << " Error! Missing required argument to '--dict-dir'!\n";
				help(argv[0]);
				return 1;
			}
			dict_dir = std::string(argv[++index]);
		}
		else{ 
			std::cout << " Error: Encountered unrecognized option '" << argv[index] << "'\n";
			return 2;
		}
		index++;
	}
	
	std::cout << " " << argv[0] << ": Generating root data structure file... ";

	StructureFile sfile;
	if(!sfile.Open(inc_dir+"Structures.h", src_dir+"Structures.cpp", dict_dir+"LinkDef.h")){ 
		std::cout << "failed\n  Error: Failed to open one of the output files!\n";
		return 3;
	}
	else if(!sfile.Process(std::string(argv[1]))){
		std::cout << "failed\n  Error: Failed to load input definitions file " << argv[1] << "!\n";
		return 4;
	}
	
	std::cout << "done\n";

	return 0;
}
