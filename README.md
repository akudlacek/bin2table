# bin2table
This program reads in a binary file and converts the data to ascii of the specified type.
It is written in C using Microsoft Visual Studio

## Arguments
ARGUMENT 1 is the source file  
ARGUMENT 2 is the endianness type:  
- little  
- big  
ARGUMENT 3 is the desired number of colums  
ARGUMENT 4 is the output data type:  
- uint8  
- int8  
- uint16  
- int16  
- uint32  
- int32  
- uint64  
- int64  
- float  
- double  

EXAMPLE  
bin2table src.bin little 19 uint16  
  