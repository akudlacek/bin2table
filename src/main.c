/*bin2table*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


/**************************************************************************************************
*                                             DEFINES
*************************************************^************************************************/
#define DEBUGGING 0

typedef union
{
	uint8_t  *_u8;
	int8_t   *_8;
	uint16_t *_u16;
	int16_t  *_16;
	uint32_t *_u32;
	int32_t  *_32;
	uint64_t *_u64;
	int64_t  *_64;
	float    *_fl;
	double   *_db;
} buffer_t;

typedef union
{
	uint8_t _uint;
	int8_t _int;
} bit8_dat_t;

typedef union
{
	uint16_t _uint;
	int16_t _int;
} bit16_dat_t;

typedef union
{
	uint32_t _uint;
	int32_t _int;
	float _flt;
} bit32_dat_t;

typedef union
{
	uint64_t _uint;
	int64_t _int;
	double _dbl;
} bit64_dat_t;

typedef enum
{
	LITTLE_ENDIAN,
	BIG_ENDIAN
} endian_t;

typedef enum
{
	UINT8,
	INT8,
	UINT16,
	INT16,
	UINT32,
	INT32,
	UINT64,
	INT64,
	FLOAT,
	DOUBLE,
} data_type_t;


/**************************************************************************************************
*                                         LOCAL PROTOTYPES
*************************************************^************************************************/
static void print_table_task(buffer_t buffer, uint64_t buffer_size_bytes, data_type_t data_type, endian_t endian, uint32_t cols);
static uint16_t ChangeEndianness_16(uint16_t value);
static uint32_t ChangeEndianness_32(uint32_t value);
static uint64_t ChangeEndianness_64(uint64_t value);

/**************************************************************************************************
*                                            VARIABLES
*************************************************^************************************************/


/**************************************************************************************************
*                                            FUNCTIONS
*************************************************^************************************************/
/******************************************************************************
*  \brief main
*
*  \note
******************************************************************************/
int main(int argc, char *argv[])
{
	//argument variables
	char *src_file_path;
	endian_t endian;
	uint32_t cols;
	data_type_t data_type;

	FILE * file_ptr;
	long file_size_bytes;
	size_t buffer_read_size_bytes;
	buffer_t buffer;

#if DEBUGGING == 1
	/*See arguments*/
	printf("argc = %d\r\n", argc);

	for(i = 0; i < argc; i++)
	{
		printf("argv[%d] = %s\r\n", i, argv[i]);
	}
#endif

	if(argc != 5)
	{
		fputs(	"Arg error\r\n"
				"bin2table src.bin endian col type\r\n"
				"                  little  #  uint8\r\n"
				"                  big        int8\r\n"
				"                             uint16\r\n"
				"                             int16\r\n"
				"                             uint32\r\n"
				"                             int32\r\n"
				"                             uint64\r\n"
				"                             int64\r\n"
				"                             float\r\n"
				"                             double\r\n", stderr);
		exit(1);

		return -1;
	}
	else
	{
		//src
		src_file_path = argv[1];

		//endian
		if(strncmp(argv[2], "little", 10) == 0) endian = LITTLE_ENDIAN;
		else if(strncmp(argv[2], "big", 10) == 0) endian = BIG_ENDIAN;
		else
		{
			fputs("Endian error", stderr);
			exit(2);
		}

		//colums
		cols = (uint32_t)atoi(argv[3]);

		//type
		if(strncmp(argv[4], "uint8", 10) == 0) data_type = UINT8;
		else if(strncmp(argv[4], "int8", 10) == 0) data_type = INT8;
		else if(strncmp(argv[4], "uint16", 10) == 0) data_type = UINT16;
		else if(strncmp(argv[4], "int16", 10) == 0) data_type = INT16;
		else if(strncmp(argv[4], "uint32", 10) == 0) data_type = UINT32;
		else if(strncmp(argv[4], "int32", 10) == 0) data_type = INT32;
		else if(strncmp(argv[4], "uint64", 10) == 0) data_type = UINT64;
		else if(strncmp(argv[4], "int64", 10) == 0) data_type = INT64;
		else if(strncmp(argv[4], "float", 10) == 0) data_type = FLOAT;
		else if(strncmp(argv[4], "double", 10) == 0) data_type = DOUBLE;
		else
		{
			fputs("Type error", stderr);
			exit(3);
		}

	}

	fopen_s(&file_ptr, argv[1], "rb");
	if(file_ptr == NULL)
	{
		fputs("File error", stderr);
		exit(4);
	}

	// obtain file size:
	fseek(file_ptr, 0, SEEK_END);
	file_size_bytes = ftell(file_ptr);
	rewind(file_ptr);

	// allocate memory to contain the whole file:
	buffer._u8 = (uint8_t*)malloc(sizeof(uint8_t)*file_size_bytes);
	if(buffer._u8 == NULL)
	{
		fputs("Memory error", stderr);
		exit(5);
	}

	// copy the file into the buffer:
	buffer_read_size_bytes = fread(buffer._u8, 1, file_size_bytes, file_ptr);
	if(buffer_read_size_bytes != file_size_bytes)
	{
		fputs("Reading error", stderr);
		exit(6);
	}

	/* the whole file is now loaded in the memory buffer. */
	print_table_task(buffer, buffer_read_size_bytes, data_type, endian, cols);

	// terminate
	fclose(file_ptr);
	free(buffer._u8);

	return 0;
}


/**************************************************************************************************
*                                         LOCAL FUNCTIONS
*************************************************^************************************************/
/******************************************************************************
*  \brief Print table task
*
*  \note
******************************************************************************/
static void print_table_task(buffer_t buffer, uint64_t buffer_size_bytes, data_type_t data_type, endian_t endian, uint32_t cols)
{
	uint64_t buffer_ind;
	uint64_t buffer_end;
	uint32_t col_ind;

	bit8_dat_t bit8_dat;
	bit16_dat_t bit16_dat;
	bit32_dat_t bit32_dat;
	bit64_dat_t bit64_dat;

	buffer_ind = 0;
	col_ind = 0;

	unsigned int tmp_int = 1;
	char *tmp_char_ptr = (char*)&tmp_int;
	endian_t this_machine_endianess;

	/*Determine endianness of your machine*/
	if(*tmp_char_ptr)
		this_machine_endianess = LITTLE_ENDIAN;
	else
		this_machine_endianess = BIG_ENDIAN;

	switch(data_type)
	{
	case UINT8:
		buffer_end = buffer_size_bytes / sizeof(buffer._u8[0]);
		break;
	case INT8:
		buffer_end = buffer_size_bytes / sizeof(buffer._8[0]);
		break;
	case UINT16:
		buffer_end = buffer_size_bytes / sizeof(buffer._u16[0]);
		break;
	case INT16:
		buffer_end = buffer_size_bytes / sizeof(buffer._16[0]);
		break;
	case UINT32:
		buffer_end = buffer_size_bytes / sizeof(buffer._u32[0]);
		break;
	case INT32:
		buffer_end = buffer_size_bytes / sizeof(buffer._32[0]);
		break;
	case UINT64:
		buffer_end = buffer_size_bytes / sizeof(buffer._u64[0]);
		break;
	case INT64:
		buffer_end = buffer_size_bytes / sizeof(buffer._64[0]);
		break;
	case FLOAT:
		buffer_end = buffer_size_bytes / sizeof(buffer._fl[0]);
		break;
	case DOUBLE:
		buffer_end = buffer_size_bytes / sizeof(buffer._db[0]);
		break;
	}

	while(buffer_ind < buffer_end)
	{
		bit8_dat._uint = buffer._u8[buffer_ind];
		bit16_dat._uint = buffer._u16[buffer_ind];
		bit32_dat._uint = buffer._u32[buffer_ind];
		bit64_dat._uint = buffer._u64[buffer_ind];

		//if the src data endian is the same as machine processing it then run normally
		if(this_machine_endianess != endian)
		{
			bit16_dat._uint = ChangeEndianness_16(bit16_dat._uint);
			bit32_dat._uint = ChangeEndianness_32(bit32_dat._uint);
			bit64_dat._uint = ChangeEndianness_64(bit64_dat._uint);
		}

		switch(data_type)
		{
		case UINT8:
			printf("%u ", bit8_dat._uint);
			break;
		case INT8:
			printf("%d ", bit8_dat._int);
			break;
		case UINT16:
			printf("%u ", bit16_dat._uint);
			break;
		case INT16:
			printf("%d ", bit16_dat._int);
			break;
		case UINT32:
			printf("%lu ", bit32_dat._uint);
			break;
		case INT32:
			printf("%ld ", bit32_dat._int);
			break;
		case UINT64:
			printf("%llu ", bit64_dat._uint);
			break;
		case INT64:
			printf("%lld ", bit64_dat._int);
			break;
		case FLOAT:
			printf("%f ", bit32_dat._flt);
			break;
		case DOUBLE:
			printf("%lf ", bit64_dat._dbl);
			break;
		}

		col_ind++;

		if(col_ind >= cols)
		{
			col_ind = 0;
			printf("\n");
		}
		buffer_ind++;
	}

	printf("\n");
}

/******************************************************************************
*  \brief ChangeEndianness_16
*
*  \note
******************************************************************************/
static uint16_t ChangeEndianness_16(uint16_t value)
{
	uint16_t result = 0;
	result |= (value & 0x00FF) << 8;
	result |= (value & 0xFF00) >> 8;
	return result;
}

/******************************************************************************
*  \brief ChangeEndianness_32
*
*  \note
******************************************************************************/
static uint32_t ChangeEndianness_32(uint32_t value)
{
	uint32_t result = 0;
	result |= (value & 0x000000FF) << 24;
	result |= (value & 0x0000FF00) << 8;
	result |= (value & 0x00FF0000) >> 8;
	result |= (value & 0xFF000000) >> 24;
	return result;
}

/******************************************************************************
*  \brief ChangeEndianness_64
*
*  \note
******************************************************************************/
static uint64_t ChangeEndianness_64(uint64_t value)
{
	uint64_t result = 0;
	result |= (value & 0x00000000000000FF) << 56;
	result |= (value & 0x000000000000FF00) << 40;
	result |= (value & 0x0000000000FF0000) << 24;
	result |= (value & 0x00000000FF000000) << 8;
	result |= (value & 0x000000FF00000000) >> 8;
	result |= (value & 0x0000FF0000000000) >> 24;
	result |= (value & 0x00FF000000000000) >> 40;
	result |= (value & 0xFF00000000000000) >> 56;
	return result;
}
