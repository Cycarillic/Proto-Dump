/*
CONTRIBUTORS :

DockerMaster

*/



#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdint.h>
#include <streambuf>


using namespace std;

// custom implementation for output stream
// 1. making a void buffer that will EAT everything

class VoidBuffer: public streambuf
{
protected:
    int overflow(int character) override {return character;}
};

//Color codes

#ifndef NOCOLOR
    #define COL_RESET "\033[0m" //Reset
    #define COL_WIRETYPE "\033[35m" //Magenta
    #define COL_LEN  "\033[93m"//Bright Yellow
    #define COL_ASCII "\033[32m" //Green
    #define COL_OFFSET "\033[90m" //Gray
    #define COL_VALUE  "\033[97m"//Bright White (What is the difference lmao)
    #define COL_NUM  "\033[94m" //Bright blue
    #define COL_ERROR "\033[30;41m" //Error (Bg - RED , Fg - BLACK)
    #define COL_HEX "\033[38;5;208m"  // Orange

#endif









int unknown_value;
uint32_t offset = 0;
uint32_t auto_break = 0xFFFFFF;
bool quiet_mode = 0;  
ostream* globalOut = &cout;

//Error handle

/*
types ; 1 - EOF while reading;
        2 - File not found
        3 - Above 32 bit integer limit
        4 - Wrong Flag Value
*/



void ERROR_HANDLE(int type,string log){
    cout << COL_ERROR << "ERROR OCCURED!\n" 
        << "ERROR TYPE : " << type << " : " << log << endl << COL_RESET ;
    ;
}


// Print Functions
void PrintMethodOffset(int pos,ofstream& out) {
    *globalOut << COL_OFFSET << "\n0x" << setw(6) << setfill('0') << hex << uppercase << pos << dec << COL_RESET << " |";
    out << "\n0x" <<setw(6) << setfill('0') << hex << uppercase << pos << dec << " |" ;
}

void PrintMethodValue(int val,ofstream& out) {
    *globalOut << COL_VALUE << hex << "-HEX >> " << COL_NUM << "0x" << setw(2) << val << COL_VALUE << " | DEC >> " << dec << COL_NUM << val << COL_RESET;
    out << hex << "-HEX >> " << "0x" << setw(2) << val << " | DECIMAL >> " << dec << val;
}

void PrintMethodLen(int val,ofstream& out) {
    int b;
    if (val <= 127) {
        b = static_cast<char>(val);
    }else{
        b = '.';
    }


    *globalOut << COL_VALUE << hex << "-HEX >> " << COL_NUM << "0x" << setw(2) << val << COL_VALUE << " | DEC >> " << dec << COL_NUM << val << " | " << COL_ASCII << static_cast<char>(b) << COL_RESET;
    out << hex << "-HEX >> " << "0x" << setw(2) << val << " | DEC >> " << dec << val << " | " << static_cast<char>(b);
}


int get_field(int val){
    return (static_cast<int>(val) >> 3);
}







//Wire types
string wire_type(int msb) {
    if (msb == 0b000) {
        return "varint";
    } else if (msb == 0b010) {
        return "len";
    } else if (msb == 0b101) {
        return "i32";
    } else {
        return "unknown";
    }
}

// All the wire type funcs

//Get value for a varint
int get_varint_val(ifstream& in,ofstream& out) {
    
    int val = in.get();
    if (val == EOF){
        ERROR_HANDLE(1,"EOF while reading varint");
        return 0;
    }
    PrintMethodOffset(offset,out);
    PrintMethodValue(val,out);
    offset++;
    return val;
}

//Get values for a I32
float get_i32_val(ifstream& in,ofstream& out) {
    int val = 0;
    int byte_count = 0;
    int start_offset = offset;
    unsigned char bytes[4];

    for (int i = 0; i < 4; ++i) {
        int b = in.get();
        if (b == EOF) {
            ERROR_HANDLE(1, "EOF while reading i32");
            return 0;
        }
        bytes[i] = static_cast<unsigned char>(b);

        PrintMethodOffset(offset,out);
        *globalOut << "-" << COL_LEN << " i32 " << COL_RESET
             << "- (" << COL_VALUE << "byte " << COL_NUM << byte_count << COL_RESET << ") >> ";
        out << "-" << " i32 " << "- (" << "byte " << byte_count  << ") >> ";
        PrintMethodValue(b,out);

        
        ++offset;
        ++byte_count;
    }

    //Big Endian
    //int32_t be = (bytes[3] << 24) |
      //        (bytes[2] << 16) |
             // (bytes[1] << 8)  |
             // (bytes[0]);

    


    float result = *(reinterpret_cast<float *>(&bytes));

    // memcpy(&result,&be,sizeof(float));
    
    
    *globalOut << "\n\t |~ Bytes in I32 >> " << COL_HEX << "0x";
    for (int i = 0; i < 4; ++i) {
        *globalOut << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(bytes[i]);
    }
    *globalOut << COL_RESET;
    *globalOut << "\n\t |~ " << COL_ASCII << "i32 value: " << dec << result << COL_RESET;

    out << "\n\t\t |~ " << "Bytes in I32 >> " << "0x" << hex << setw(8) << bytes;
    out << "\n\t\t |~ " << "i32 value: " << dec << result << "\n";

    return result;
}



//Get value for a len
void get_len_value(ifstream& in, int len,ofstream& out) {
    string ascii;
    int byte_count = 0;
    for (int i = 0; i < len; ++i) {
        
        int b = in.get();
        if (b == EOF) {
            ERROR_HANDLE(1,"EOF while reading len");
            break;
        }
        unsigned char wire_type = b & 0x07;


        PrintMethodOffset(offset,out);
        *globalOut << "-" << COL_LEN << " len " << COL_RESET << "- (" << COL_VALUE << "byte " << COL_NUM << byte_count << COL_RESET << ") >> " ;
        
        out << "-" << " len " << "- (" << "byte " << byte_count << ") >> " ;
        PrintMethodLen(b,out);
        offset++;
        byte_count++;
        if (b >= 32 and b <= 126 and b != '\n' and b != '\r') {
            ascii += static_cast<char>(b);
        }else if (b == 0x00){
            ascii += '.';
        }
         else {
            ascii += '.';
        }


        
    }
    *globalOut << "\n" << "\t |~ "<< COL_OFFSET << "DECODED : \n" << COL_ASCII << ascii << COL_RESET ;
    out << "\n" << "\t\t |~ " << "DECODED : \n/* NOTE -> Bytes Greater than 127 are Represnted by '.' in ASCII */\n\n" << ascii << "\n\n" ;
}

//Flag vars
string out_file_name;


//MAIN
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "USAGE => \nproto <filename> <flags>" << endl;
        return 1;
    }

     //Parsing the Flags and stuff
    for (int a = 0; a < argc; ++a) {
        string current_arg = string(argv[a]);
        if (current_arg == "-h" || current_arg == "--help"){
            cout << R"(
PROTO DUMP HELP : 

VERSION | 0.5

FLAGS/OPTIONS :
    -o : Save the dump to output file | -o <OutputFileName>
    -s : Automatically Terminate the dumping at a offset (after the current pasring of wire types end) | -s <Offset>
    -q/--quiet : quiet mode does not print parsed bytes to console (Increases Performace, Recommended for big/huge files).
    -v/--version : Get the Version.
Example :

proto hiro.scl -o hiro.scl.dump -s 0xFF
*Terminates After offset 0xFF. NOTE : The Break limit will be crossed if a wiretype if being dumped.*

Notes:
  - This tool dumps protobuf wire types.
  - Colored terminal output improves readability.
  - ASCII decoding is shown for length fields.
                
)";
                return 0;
        }else if(current_arg == "-v" || current_arg == "--version"){
            cout << "VERSION | 0.5";
            return 0;
        }else if (current_arg == "-o" || current_arg == "--outfile"){
            out_file_name = argv[a + 1];
        }else if (current_arg == "-s" || current_arg == "--stop") {
            if (++a < argc) {
                auto_break = stoi(argv[++a], nullptr, 16);
            } else {
                ERROR_HANDLE(4, "Missing value for -s flag");
                return 1;
            }
        }else if (current_arg == "-q" || current_arg == "--quiet") {
            quiet_mode = 1;
        }
    }
    
    VoidBuffer voidBuffer;
    ostream quietOut(&voidBuffer);

    if(quiet_mode)
    {
        globalOut = &quietOut;
    }


    cout << COL_ASCII << "Attempting to open " << argv[1] << COL_RESET << endl;


    //Open file stuff

    ifstream in(argv[1], ios::binary);

    if (!in) {
        ERROR_HANDLE(2,"FILE NOT FOUND!" );
        *globalOut << COL_ERROR << argv[1] << COL_RESET;
        return 1;
    }

    cout << COL_ASCII << "File Succesfully opened !"<< COL_RESET << endl;


   

    

    ofstream out(out_file_name);

    cout << "Auto break at offset: 0x" << hex << auto_break << dec << " (" << auto_break << ")" << endl;

    

    
    

    
    int byte;

    

    while ((byte = in.get()) != EOF) {

        if (byte != 0x00){

            int old_offset = offset;offset++;
            unsigned char bit = static_cast<unsigned char>(byte);
            unsigned char bit3 = bit & 0x07;
            int type;



            if (bit3 == 0b000){
                type = 1; //varint
            }else if (bit3 == 0b010){
                type = 2; //len
            }else if (bit3 == 0b101){
                type = 3; //I32
            }else{
                type = -1; //Unknown
            }

            
            *globalOut << COL_OFFSET << "0x" << uppercase << setw(6) << setfill('0') << hex << old_offset << COL_RESET << " ~ "
                << hex << COL_HEX << "0x" << setw(2) << setfill('0') << static_cast<int>(bit) << nouppercase
                << "  >> " << dec << COL_WIRETYPE << wire_type(bit3) << " | " << type << COL_RESET ;
            
            
            out << "0x" << setw(6) << setfill('0') << hex << old_offset << " ~ "
                << hex  << "0x" << setw(2) << setfill('0') << static_cast<int>(bit)
                << "  >> " << dec  << wire_type(bit3) << " | " << type  ;


            if (bit3 == 0b000) {  // varint
                
                int field = get_field(byte);
                *globalOut << "\n\t |-" << COL_LEN << "Field ~ " << field ;
                out << "\n\t\t |-" << "Field ~ " << field ;
                get_varint_val(in,out);
                out << "\n";

            } else if (bit3 == 0b101) {  // i32
                get_i32_val(in,out);

            } else if (bit3 == 0b010) {  // len
                int len = in.get();
                if (len == EOF) {
                    *globalOut << "\nERROR: EOF while reading length\n";
                    out << "\nERROR: EOF while reading length\n";
                    break;
                }
                
                PrintMethodOffset(offset,out);
                //Output the length
                *globalOut << COL_LEN << "- Length >> " << COL_NUM<< len << COL_RESET;
                out << "- Length >> " << len ;
                offset++;
                get_len_value(in, len,out);
                

            } else {
                unknown_value++;
                *globalOut << "\n";
                out << "\n";
            }
            *globalOut << endl;
            if (offset >= auto_break){
                *globalOut << "\nAuto-Break Called.\n";
                break;
            }
        }else{
            offset++;
        }
        
    }

    
    if (out.is_open() == true){
        cout << COL_NUM << endl << argv[1] << " Successfully DUMPED as " << out_file_name  << "\n" << COL_RESET ;
    }
    
    
    
    out.close();

    in.close();
    return 0;
}

