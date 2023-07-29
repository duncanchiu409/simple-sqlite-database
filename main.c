#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

// written by duncanchiu409 credited to cstack

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)
// understand what is # define function in C
// https://www.educba.com/sharp-define-in-c/

typedef enum{
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNISED_COMMAND
} MetaCommandResult;

typedef enum{
    STATEMENT_SELECT,
    STATEMENT_INSERT
} StatementType;

typedef enum{
    PREPARE_SUCCESS,
    PREPARE_UNRECOGNISED_STATEMENT,
    PREPARE_SYNTAX_ERROR,
    PREPARE_STRING_TOO_LONG,
    PREPARE_ID_NEGATIVE
} PrepareResult;

typedef enum{
    EXECUTE_SUCCESS,
    EXECUTE_TABLE_FULL
} ExecuteResult;

typedef struct{
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE + 1];
    char email[COLUMN_EMAIL_SIZE + 1];
} Row;

typedef struct{
    StatementType type;
    Row row_inserting;
} Statement;

typedef struct{
    char* buffer;
    size_t buffer_length;
    ssize_t input_length;
} InputBuffer;

const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);

const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t TOTAL = EMAIL_OFFSET + EMAIL_SIZE;

void serialise_row(Row* source, void* destination){
    memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
    memcpy(destination + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
    memcpy(destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

void deserialise_row(void* source, Row* destination){
    memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}

const uint32_t PAGE_SIZE = 4096; // 4kb
#define TABLE_MAX_PAGES 100
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / TOTAL;
const uint32_t TABLE_MAX_ROWS = TABLE_MAX_PAGES * ROWS_PER_PAGE;

typedef struct{
    int file_descriptor;
    uint32_t file_length;
    void* pages[TABLE_MAX_PAGES];
} Pager;

typedef struct{
    uint32_t number_of_rows;
    Pager* pager;
} Table;

Pager* pager_open(const char* filename){
    int file_descriptor = open(filename, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);

    if(file_descriptor == -1){
        printf("Unable to open file\n");
        exit(EXIT_FAILURE);
    }

    off_t file_length = lseek(file_descriptor, 0, SEEK_END);

    if(file_length == -1){
        printf("Unable to set fileDescriptor offset\n");
        exit(EXIT_FAILURE);
    }

    Pager* pager = malloc(sizeof(Pager));
    pager->file_descriptor = file_descriptor;
    pager->file_length = file_length;
    for(int i = 0; i < TABLE_MAX_PAGES; i++){
        pager->pages[i] = NULL;
    }
    return pager;
}

Table* new_db(const char* filename){
    Pager* pager = pager_open(filename);
    uint32_t number_rows = pager->file_length / TOTAL;

    Table* table = malloc(sizeof(Table));
    table->number_of_rows = number_rows;
    table->pager = pager;
    return table;
}

void pager_flush(Pager* pager, uint32_t i, uint32_t page_size){
    if(pager->pages[i]==NULL){
        printf("Tried to flush null page.\n");
        exit(EXIT_FAILURE);
    }

    off_t offset = lseek(pager->file_descriptor, i*PAGE_SIZE, SEEK_SET);
    if(offset == -1){
        printf("Error seeking: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    ssize_t bytes_written = write(pager->file_descriptor, pager->pages[i], page_size);
    if(bytes_written == -1){
        printf("Error writing: %d\n", errno);
        exit(EXIT_FAILURE);
    }
}

void close_db(Table* table){
    Pager* pager = table->pager;
    uint32_t number_of_full_pages = table->number_of_rows / ROWS_PER_PAGE;
    for(uint32_t i = 0; i < number_of_full_pages; i++){
        if(pager->pages[i]!=NULL){
            pager_flush(pager, i, PAGE_SIZE);
            free(pager->pages[i]);
            pager->pages[i] = NULL;
        }
    }

    // There might be partial pages
    uint32_t number_of_additional_rows = table->number_of_rows % ROWS_PER_PAGE;
    if(number_of_additional_rows > 0){
        uint32_t page_number = number_of_full_pages;
        if(pager->pages[page_number]!=NULL){
            pager_flush(pager, page_number, number_of_additional_rows*TOTAL);

        }
    }

    int result = close(pager->file_descriptor);
    if(result == -1){
        printf("Error closing db file.\n");
        exit(EXIT_FAILURE);
    }

    // Make sure the cache database in memory is wiped
    for(uint32_t i = 0; i < TABLE_MAX_PAGES; i++){
        void* page = pager->pages[i];
        if(page){
            free(page);
            pager->pages[i] = NULL;
        }
    }
    free(pager);
    free(table);
}

void* get_page(Pager* pager, uint32_t page_number){
    if(page_number >= TABLE_MAX_PAGES){
        printf("Tried to fetch page number out of bounds. %d > %d\n", page_number, TABLE_MAX_PAGES);
        exit(EXIT_FAILURE);
    }
    else{
        if(pager->pages[page_number] == NULL){
            void* page = malloc(PAGE_SIZE);
            uint32_t number_of_pages = pager->file_length / PAGE_SIZE;

            if(pager->file_length % PAGE_SIZE){
                number_of_pages += 1;
            }

            if(page_number <= number_of_pages){
                lseek(pager->file_descriptor, number_of_pages*PAGE_SIZE, SEEK_SET);
                ssize_t bytes_read = read(pager->file_descriptor, page, PAGE_SIZE);
                if(bytes_read == -1){
                    printf("Error reading file: %d\n", errno);
                    exit(EXIT_FAILURE);
                }
            }
            pager->pages[page_number] = page;
        }
    }
    return pager->pages[page_number];
}

void* row_slot(Table* table, uint32_t row_number){
    uint32_t page_number = row_number / ROWS_PER_PAGE;
    void* page = get_page(table->pager, page_number);

    uint32_t row_offset = row_number % ROWS_PER_PAGE;
    uint32_t bytes_offset = row_offset * TOTAL;
    return page + bytes_offset;
}

InputBuffer* new_Input_Buffer(){
    InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(InputBuffer));
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;
    return input_buffer;
}

void close_Input_Buffer(InputBuffer* input_buffer){
    free(input_buffer->buffer);
    free(input_buffer);
}

void print_prompt(){
    printf("sqlite > ");
}

void print_row(Row* row){
    printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}

void read_input(InputBuffer* input_buffer){
    ssize_t bytes_read = getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);

    if(bytes_read <= 0){
        printf("Error reading input\n");
        close_Input_Buffer(input_buffer);
        exit(EXIT_FAILURE);
    }

    input_buffer->input_length = bytes_read - 1;
    input_buffer->buffer[bytes_read - 1] = '\0'; // replace newline char to end of line char
}

MetaCommandResult do_meta_command(InputBuffer* input_buffer, Table* table){
    if(strcmp(input_buffer->buffer, ".exit") == 0){
        close_Input_Buffer(input_buffer);
        close_db(table);
        exit(EXIT_SUCCESS);
    }
    else{
        return META_COMMAND_UNRECOGNISED_COMMAND;
    }
}

PrepareResult prepare_insert(InputBuffer* input_buffer, Statement* statement){
    char* keyword = strtok(input_buffer->buffer, " ");
    char* id_string = strtok(NULL, " ");
    char* username = strtok(NULL, " ");
    char* email = strtok(NULL, " ");

    if(id_string == NULL || username == NULL || email == NULL){
        return PREPARE_SYNTAX_ERROR;
    }

    int id = atoi(id_string);
    if(id < 0){
        return PREPARE_ID_NEGATIVE;
    }
    if(strlen(username) > COLUMN_USERNAME_SIZE){
        return PREPARE_STRING_TOO_LONG;
    }
    if(strlen(email) > COLUMN_EMAIL_SIZE){
        return PREPARE_STRING_TOO_LONG;
    }

    statement->type = STATEMENT_INSERT;
    statement->row_inserting.id = id;
    strcpy(statement->row_inserting.username, username);
    strcpy(statement->row_inserting.email, email);

    return PREPARE_SUCCESS;
}

PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement){
    if(strncmp(input_buffer->buffer, "insert", 6) == 0){
        return prepare_insert(input_buffer, statement);
    }
    else if(strcmp(input_buffer->buffer, "select") == 0){
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }
    else{
        return PREPARE_UNRECOGNISED_STATEMENT;
    }
}

ExecuteResult execute_insert(Statement* statement, Table* table){
    if(table->number_of_rows == TABLE_MAX_ROWS){
        return EXECUTE_TABLE_FULL;
    }

    Row* row_inserting = &(statement->row_inserting);

    serialise_row(row_inserting, row_slot(table, table->number_of_rows));
    table->number_of_rows += 1;

    return EXECUTE_SUCCESS;
};

ExecuteResult execute_select(Statement* statement, Table* table){
    Row row;
    for(uint32_t i = 0; i < table->number_of_rows; i++){
        deserialise_row(row_slot(table, i), &row);
        print_row(&row);
    }
    return EXECUTE_SUCCESS;
}

ExecuteResult execute_statement(Statement* statement, Table* table){
    switch(statement->type)
    {
        case(STATEMENT_INSERT):
            return execute_insert(statement, table);
            break;
        case(STATEMENT_SELECT):
            return execute_select(statement, table);
            break;
        default:
            printf("Unrecognized StatementType.\n");
            break;
    }
}

int main(int argc, char* argv[]){
    if(argc < 2){
        printf("Must supply a database filename\n");
        exit(EXIT_FAILURE);
    }

    Table* table = new_db(argv[1]);
    InputBuffer* input_buffer = new_Input_Buffer();
    while(1){
        print_prompt();
        read_input(input_buffer);

        // understand keyword 'continue'
        // https://www.geeksforgeeks.org/difference-between-break-and-continue-statement-in-c/#:~:text=The%20continue%20statement%20is%20not%20used%20with%20the%20switch%20statement,from%20the%20loop%20construct%20immediately.

        // Meta-commands
        if(input_buffer->buffer[0] == '.'){
            switch(do_meta_command(input_buffer, table))
            {
            case(META_COMMAND_SUCCESS):
                break;
            case(META_COMMAND_UNRECOGNISED_COMMAND):
                printf("Unrecognized command '%s'.\n", input_buffer->buffer);
                break;
            default:
                printf("Unrecognized MetaCommandResult.\n");
                break;
            }
        }
        // SELECT or INSERT commands
        else{
            Statement statement;
            switch (prepare_statement(input_buffer, &statement))
            {
            case (PREPARE_SUCCESS):
                switch(execute_statement(&statement, table)){
                    case(EXECUTE_SUCCESS):
                        printf("Executed.\n");
                        break;
                    case(EXECUTE_TABLE_FULL):
                        printf("Error: Table full.\n");
                        break;
                }
                break;
            case (PREPARE_SYNTAX_ERROR):
                printf("Syntax error. Could not parse statement.\n");
                break;
            case (PREPARE_UNRECOGNISED_STATEMENT):
                printf("Unrecognized command '%s'.\n", input_buffer->buffer);
                break;
            case (PREPARE_STRING_TOO_LONG):
                printf("String is too long.\n");
                break;
            case (PREPARE_ID_NEGATIVE):
                printf("ID must be positive.\n");
                break;
            default:
                printf("Unrecognized PrepareResult.\n");
                break;
            }
        }
    }
    return 0;
}
