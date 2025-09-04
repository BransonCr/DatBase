# Visual README

## Project Notes & Progress

> **2025-08-23**
>
> Hello!
>
> This is going to be an explaination of what this is! this is mainly for me, but I find that writing things down is very important in the nature of learning.
>
> I will be updating this on what portion of the code we are at!!!
>
> In the dotted lines we have the first boilderplate introduction to sqlite, which is a representation of SQLite, a widely used database engine.
>
> * **Database**: A system that store data in an organized way! (Tables, rows and columns)
> * **SQLite**: A small, fast, self-contained SQL database engine that:
>
> 1. Runs inside your app (no seperate server needed)
> 2. Uses a single file to store the whole database
> 3. Very portable
>
> It's used pretty much everywhere, IOS, Firefox, Chrome.
>
> As of below we have a car, you can turn it on and it will start, but there is no gas pedal, any type of fuctionality is missing as of now.
>
> No commands work, you can only exit and start it.
>
> As I go, I won't be explaining every line of code, I don't have to define what a struct is in C
>
> Or why I serialize/deserialize rows and why there is a function for that

---

> **2025-08-25**
>
> We have now added some interesting features so this very simple database. As of now we only insert id's Usernames, and emails:
>
> | column   | size (bytes) | offset |
> | -------- | ------------ | ------ |
> | id       | 4            | 0      |
> | username | 32           | 4      |
> | email    | 255          | 36     |
> | total    | 291          |        |
>
> Our page size right now is only 4 kilobytes, it's the same size as a page used in the virtual memory of most computer architectures.
>
> The plan so far is to:
>
> * Store rows in blocks of memory called pages
> * Each page stores as many rows as it can fit
> * Rows are serialized into a compact representation with each page
> * Pages are only allocated as needed
> * Keep a fixed-size array of pointers to pages
>
> ```c
> const uint32_t PAGE_SIZE = 4096;
> const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
> const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;
>
> typedef struct {
>    uint32_t num_rows;
>    void* pages[TABLE_MAX_PAGES];
> } Table;
> ```
>
> Preparing our statements no longer uses `scanf()` which could cause a buffer overflow if the string it's reading is larger than the buffer it's reading into.
>
> `strtok()` fixes this issue in our `prepare_insert()` function.
>
> ```c
> PrepareResult prepare_insert(InputBuffer* input_buffer, Statement* statement){
>    statement->type = STATEMENT_INSERT;
>
>    char* keyword = strtok(input_buffer->buffer, " ");
>    char* id_string = strtok(NULL, " " );
>    char* username = strtok(NULL," ");
>    char* email = strtok(NULL, " ");
>
>    if(id_string = NULL || username == NULL || email == NULL){
>        return PREPARE_SYNTAX_ERROR;
>    }
>
>    int id = atoi(id_string);
>    if(id<0){
>        return PREPARE_NEGATIVE_ID;
>    }
>    if(strlen(email) > COLUMN_EMAIL_SIZE) {
>        return PREPARE_STRING_TOO_LONG;
>    }
>
>    statement->row_to_insert.id = id;
>    strcpy(statment->row_to_insert.username,username);
>    strcpy(statment->row_to_insert.email,email);
>
>    return PREPARE_SUCCESS
> }
> ```
>
> We also have our **SYNTAX ERROR HANDLER**, still better than Java.
>
> ```c
> Statement statement;
> switch(prepare_statement(input_buffer, &statement)){
>    case(PREPARE_SUCCESS):
>        break;
>    case(PREPARE_NEGATIVE_ID):
>        printf("ID must be positive.\n");
>        continue;
>    case(PREPARE_STRING_TOO_LONG):
>        printf("String is too long.\n");
>        continue;
>    case(PREPARE_SYNTAX_ERROR):
>        printf("Syntax error. Could not parse statement.");
>        continue;
>    case(PREPARE_UNRECOGNIZED_STATEMENT):
>        printf("Unrecognized keyword at start of '%s'.\n", input_buffer->buffer);
>        continue;
> }
> ```
>
> Ruby tests (`spec_helper`) are also written up:
>
> ```ruby
> ```

describe 'database' do
def run\_script(commands)
raw\_output = nil
IO.popen("./db", "r+") do |pipe|
commands.each do |command|
pipe.puts command
end

>

```
  pipe.close_write
```

>

```
  raw_output = pipe.gets(nil)
```

```
end
raw_output.split("\n")
```

end

it 'inserts and retrieves a row' do
result = run\_script(\[
"insert 1 user1 [person1@example.com](mailto:person1@example.com)",
"select",
".exit",
])
expect(result).to match\_array(\[
"db > Executed.",
"db > (1, user1, [person1@example.com](mailto:person1@example.com))",
"Executed.",
"db > ",
])
end
end

> ```
> ```

---

Additional sections describe **B-Tree implementation**, **leaf node splitting**, **internal node splitting**, and **pager abstraction** with detailed code blocks and comments.

---

## Current Status

* Implemented basic insert/select functionality
* Pager abstraction working
* B-tree partially implemented
* Leaf node split handled
* Internal node split logic in progress
* Tests: 11 total, 2 passing (to be fixed later)

---

## Next Steps

* Fix parent node updates after splits
* Complete internal node split logic
* Ensure all tests pass

---
