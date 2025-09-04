/*** 2025-08-23 ***/
Hello!
This is going to be an explaination of what this is! this is mainly for me, but I find that writing things down is very important in the nature of learning.
I will be updating this on what portion of the code we are at!!!
In the dotted lines we have the first boilderplate introduction to sqlite, which is a representation of SQLite, a widely used database engine.
Database: A system that store data in an organized way! (Tables, rows and columns)
SQLite: A small, fast, self-contained SQL database engine that:
    1.Runs inside your app (no seperate server needed),
    2.Uses a single file to store the whole database
    3.Very portable
It's used pretty much everywhere, IOS,Firefox,Chrome.

As of below we have a car, you can turn it on and it will start, but there is no gas pedal, any type of fuctionality is missing as of now.
No commands work, you can only exit and start it.

As I go, I won't be explaining every line of code, I don't have to define what a struct is in C
Or why I serialize/deserialize rows and why there is a function for that

/*** 2025-08-25 ***/
We have now added some interesting features so this very simple database. As of now we only insert id's Usernames, and emails which looks like this 
| column   | size (bytes) | offset |
|----------|--------------|--------|
| id       | 4            | 0      |
| username | 32           | 4      |
| email    | 255          | 36     |
| total    | 291          |        |


Our page size right now is only 4 kilobytes, it's the same size as a page used in the virtual memory of most computer architectures. Meaning one page in our database
corresponds to one page used by the operating system.
The plan so far is to:
    Store rows in blocks of memory called pages
    Each page stores as many rows as it can fit
    Rows are serialized into a compact representation with each page
    Pages are only allocated as needed
    Keep a fixed-size array of pointers to pages

```c
const uint32_t PAGE_SIZE = 4096;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

typedef struct {
	uint32_t num_rows;
	void* pages[TABLE_MAX_PAGES];
} Table;
```

Preparing our statements no longer uses scanf() which could cause a buffer overflow if the string
it's reading is larger than the buffer it's reading into. Strtok() fixes this issue in our prepare_insert() function
Calling strtok successively on the input buffer breaks it into substrings by inserting a null character whenever it reaches a delimiter (space, in our case). It returns a pointer to the start of the substring.

We can call strlen() on each text value to see if it’s too long.
```c
PrepareResult prepare_insert(InputBuffer* input_buffer, Statement* statement){
    statement->type = STATEMENT_INSERT;

    char* keyword = strtok(input_buffer->buffer, " ");
    char* id_string = strtok(NULL, " " );
    char* username = strtok(NULL," ");
    char* email = strtok(NULL, " ");

    if(id_string = NULL || username == NULL || email == NULL){
        return PREPARE_SYNTAX_ERROR;
    }

    int id = atoi(id_string);
    if(id<0){
        return PREPARE_NEGATIVE_ID;
    }
    if(strlen(email) > COLUMN_EMAIL_SIZE) {
        return PREPARE_STRING_TOO_LONG;
    }

    statement->row_to_insert.id = id;
    strcpy(statment->row_to_insert.username,username);
    strcpy(statment->row_to_insert.email,email);

    return PREPARE_SUCCESS
}
```

We also have our SYNTAX ERROR HANDLER, still better than java.
Side tangent, why can't we just stay in C? why does everything in Java need to be so repetitive.
also EVERYTHING is a class (except for when it's not) I feel like im tip toeing around a problem when I code in java wheras in C it's like all meat no filler, delicious

I only complain about java when I don't have to maintain any code, I'm not building enterprise-scale applications lol
```c
Statement statement;
	switch(prepare_statement(input_buffer, &statement)){
		case(PREPARE_SUCCESS):
			break;
        case(PREPARE_NEGATIVE_ID):
            printf("ID must be positive.\n");
            continue;
        case(PREPARE_STRING_TOO_LONG):
            printf("String is too long.\n");
            continue;
		case(PREPARE_SYNTAX_ERROR):
			printf("Syntax error. Could not parse statement.");
			continue;
		case(PREPARE_UNRECOGNIZED_STATEMENT):
			printf("Unrecognized keyword at start of '%s'.\n", input_buffer->buffer);
			continue;
	}
```
Now we will talk about spec_helper, some ruby tests that are written up so I don't have to keep testing the code
it's pretty readable, (like python, except when its not) I won't go in depth
```c
describe 'database' do
  def run_script(commands)
    raw_output = nil
    IO.popen("./db", "r+") do |pipe|
      commands.each do |command|
        pipe.puts command
      end

      pipe.close_write

      # Read entire output
      raw_output = pipe.gets(nil)
    end
    raw_output.split("\n")
  end

  it 'inserts and retrieves a row' do
    result = run_script([
      "insert 1 user1 person1@example.com",
      "select",
      ".exit",
    ])
    expect(result).to match_array([
      "db > Executed.",
      "db > (1, user1, person1@example.com)",
      "Executed.",
      "db > ",
    ])
  end
```
Implementing a B-tree in this database. As of now we aren't using them, which is causing some headaches with memory management
Using a B-Tree is faster (log time), inserting/deleting a value you've already found is fast, and traversing a range of values it fast
(unless the tree is not a B-Tree in which it could resemble a linked list which is O(n) time to traverse)
 	Unsorted Array of rows	Sorted Array of rows	Tree of nodes
Pages contain	only data	only data	metadata, primary keys, and data
|                     | Unsorted Array of rows | Sorted Array of rows | Tree of nodes |
|---------------------|------------------------|----------------------|---------------|
| Rows per page       | more                   | more                 | fewer         |
| Insertion           | O(1)                   | O(n)                 | O(log(n))     |
| Deletion            | O(n)                   | O(n)                 | O(log(n))     |
| Lookup by id        | O(n)                   | O(log(n))            | O(log(n))     |


Implementation could be an array (slow for insertions) so lets use a tree structure for that juice O(log(n)) time!
Only thing is we can't use a binary search to find values because not an array. 

Leaf nodes and internal nodes have different layouts. Let’s make an enum to keep track of node type:
```c
typedef enum { NODE_INTERNAL, NODE_LEAF } NodeType;
```

To Reiterate, the Pager is an abstraction that when asked for page number x, the pager gives us back
a block of memory. First looking in the cache then on cache miss copies from the disk to memory

Each node will correspond to one page. Internal nodes will point to their children by storing the page number that stores the child. 
The btree asks the pager for a particular page number and gets back a pointer into the page cache. 
Pages are stored in the database file one after the other in order of page number.

Nodes need to store some metadata in a header at the beginning of the page. 
Every node will store what type of node it is, whether or not it is the root node, 
and a pointer to its parent (to allow finding a node’s siblings). 
I define constants for the size and offset of every header field:



```c
/* Common Node Layout */
/* Each node corresponds to one page, Internal nodes point to children by storing the page number that store the child
   The btree asks the pager for partic page number and gets back a pointer into the page cache. Pages are stored 
   in the databse file one after theo ther in order of page number.*/

/*Nodes need to store some metadata in a header at the beginning of the page.*/
const uint32_t NODE_TYPE_SIZE = sizeof(uint8_t);
const uint32_t NODE_TYPE_OFFSET = 0;
const uint32_t IS_ROOT_SIZE = sizeof(uint8_t);
const uint32_t IS_ROOT_OFFSET = sizeof( NODE_TYPE_SIZE);
const uint32_t PARENT_POINTER_SIZE = sizeof(uint32_t);
const uint32_t PARENT_POINTER_OFFSET = IS_ROOT_OFFSET + IS_ROOT_SIZE;
const uint8_t COMMON_NODE_HEADER_SIZE = NODE_TYPE_SIZE + IS_ROOT_SIZE + PARENT_POINTER_SIZE;


/* Leaf Node Header Layout */
/* Leaf nodes also need to store how many "cells' they contrain. A cell is a key/value pair"*/
const uint32_t LEAF_NODE_NUM_CELLS_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_NUM_CELLS_OFFSET = COMMON_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_HEADER_SIZE = COMMON_NODE_HEADER_SIZE + LEAF_NODE_NUM_CELLS_SIZE;

/* Leaf Node Body Layout */
const uint32_t LEAF_NODE_KEY_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_KEY_OFFSET = 0;
const uint32_t LEAF_NODE_VALUE_SIZE = ROW_SIZE;
const uint32_t LEAF_NODE_VALUE_OFFSET =
    LEAF_NODE_KEY_OFFSET + LEAF_NODE_KEY_SIZE;
const uint32_t LEAF_NODE_CELL_SIZE = LEAF_NODE_KEY_SIZE + LEAF_NODE_VALUE_SIZE;
const uint32_t LEAF_NODE_SPACE_FOR_CELLS = PAGE_SIZE - LEAF_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_MAX_CELLS =
    LEAF_NODE_SPACE_FOR_CELLS / LEAF_NODE_CELL_SIZE;
```
Based on these constants, here's what the layout of a leaf node looks like currently
leaf-node-format.png

Now I'm sure you all read through and saw my rant with java earlier but I will admit, this is
where Java shines, why create getters/setters in the same file, why not just create a tree class
that stores all this information and create an object in the db.java file, where everything is all
neat and abstracted from the user, well because C doesn't have that functionality unfortunately. Anyways...

I also won't explain the implementation of the tree structure, because I'm sure I will remember :)


Now, I'm implementing the splitting of the leaf node, when we run out of room basically, as of the
SQLite Database system design,

  If there is no space on the leaf node, we would split the existing entries residing there and the new one (being inserted) 
  into two equal halves: lower and upper halves. (Keys on the upper half are strictly greater than those on the lower half.)
   We allocate a new leaf node, and move the upper half into the new node.


```c
/*** Leaf Node operations ***/
void leaf_node_split_and_insert(Cursor* cursor, uint32_t key, Row* value){
    /* Create a new node and move half the cells over.
    Insert the new value in one of the two node .
    Update parent or create a new parent.*/
    void* old_node = get_page(cursor->table->pager, cursor->page_num);
    uint32_t new_page_num = get_unused_page_num(cursor->table->pager);
    void* new_node = get_page(cursor->table->pager, new_page_num);
    initialize_leaf_node(new_node);

    /*All existing keys plus new key should be divided
    evenly between old (left) and new (right) nodes.
    starting from the right, move each key to correct position*/

    for(uint32_t i = LEAF_NODE_MAX_CELLS; i >= 0; i--){
        void* destination_node;
        if(i >= LEAF_NODE_MAX_CELLS){
            destination_node = new_node;
        }else {
            destination_node = old_node;
        }

        uint32_t index_within_node = i % LEAF_NODE_LEFT_SPLIT_COUNT;
        void* destination = leaf_node_cell(destination_node, index_within_node);

        if(i == cursor->cell_num){
            serialize_row(value, destination);
        }else if(i > cursor->cell_num){
            memcpy(destination, leaf_node_cell(old_node, i - 1), LEAF_NODE_CELL_SIZE);
        } else {
            memcpy(destination, leaf_node_cell(old_node, i), LEAF_NODE_CELL_SIZE);
        }
    }
    /* Update cell count on both leaf nodes */
    *(leaf_node_num_cells(old_node)) = LEAF_NODE_LEFT_SPLIT_COUNT;
    *(leaf_node_num_cells(new_node)) = LEAF_NODE_RIGHT_SPLIT_COUNT;
    

    if (is_node_root(old_node)) {
        return create_new_root(cursor->table, new_page_num);
    } else {
        printf("Need to implement updating parent after split\n");
        exit(EXIT_FAILURE);
    }
}
```
Keeping the tree balanced, we evenly distribute cells between the two new
nodes. If a leaf node can hold N cells, then during a split we neet to distribute
N+1 cells between tro nodes (N original cells plus one new one).


Creating a new root node?
The SQLite def 
  Let N be the root node. First allocate two nodes, say L and R. Move lower half of N into L and the upper half into R. Now N is empty. 
  Add 〈L, K,R〉 in N, where K is the max key in L. Page N remains the root. Note that the depth of the tree has increased by one,
  but the new tree remains height balanced without violating any B+-tree property.

Since we've already allocated the right child and move the upper half into it. Our function takes the right child as input 
and allocates a new page to store the left child
```c
void create_new_root(Table* table, uint32_t right_child_page_num){
    /*
    Handle Splitting the root. Old root copied to new page, becomes left child
    address of right child passed in 
    Re-initialize root page to contain the new root node
    new root node points to two children
    */

    void* root = get_page(table->pager, table->root_page_num);
    void* right_child = get_page(table->pager, right_child_page_num);
    uint32_t left_child_page_num = get_unused_page_num(table->pager);
    void * left_child = get_page(table->pager, left_child_page_num);
    
    /*LEft child has data copied from the old root*/
    memcpy(left_child, root, PAGE_SIZE);
    set_node_root(left_child, false);

    /*Root node is anew internal node with one key and two children*/
    initialize_internal_node(root);
    set_node_root(root,true);
    *internal_node_num_keys(root) = 1;
    *internal_node_child(root, 0) = left_child_page_num;
    uint32_t left_child_max_key = get_node_max_key(left_child);
    *internal_node_key(root, 0) = left_child_max_key;
    *internal_node_right_child(root) = right_child_page_num;
}
```

Notice our huge branching factor. Because each child pointer / key pair is so small, we can fit 510 keys and 511 child pointers in each internal node.
 That means we’ll never have to traverse many layers of the tree to find a given key!

internal node layers, max # leaf nodes, Size of all leaf nodes
| internal node layers | max # leaf nodes    | Size of all leaf nodes  |
|----------------------|---------------------|-------------------------|
| 0                    | 511^0 = 1           | 4 KB                    |
| 1                    | 511^1 = 512         | ~2 MB                   |
| 2                    | 511^2 = 261,121     | ~1 GB                   |
| 3                    | 511^3 = 133,432,831 | ~550 GB                 |



As of now 
With 15 entries, our btree consists of one internal node and two leaf nodes.
To scan the entire table, we need to jump to the second leaf node after we reach the end of the first. 
To do that, we’re going to save a new field in the leaf node header called “next_leaf”, 
which will hold the page number of the leaf’s sibling node on the right. 
The rightmost leaf node will have a next_leaf value of 0 to denote no sibling (page 0 is reserved for the root node of the table anyway).

At this point in my writing, I have 11 tests, only 2 of which pass, this is something I will do later...
Our next step is to handle fixing up the parent node after splitting a leaf.
 
NOte I did that, it was hard to understand but after literally drawing it, it was much simpler,
But now I'm implementing a step above that, what happens when we split internal nodes which are unable 
to accomodate new keys. This would result in a new top node with 2 new internal nodes and appointing the children/
leaf nodes correctly to maintain continuity.
in which
    1. Create a sibling node to store (n-1)/2 of the original node’s keys
    2. Move these keys from the original node to the sibling node
    3. Update the original node’s key in the parent to reflect its new max key after splitting
    4. Insert the sibling node into the parent (could result in the parent also being split)
Here is the new method internal_node_split_and_insert
```c
void internal_node_insert(Table* table, uint32_t parent_page_num, uint32_t child_page_num){
    /* Add a new child/key pair to parent that corresponds to child */

    void* parent = get_page(table->pager, parent_page_num);
    void* child = get_page(table->pager, child_page_num);
    uint32_t child_max_key = get_node_max_key(table->pager, child);
    uint32_t index = internal_node_find_child(parent, child_max_key);

    uint32_t original_num_keys = *internal_node_num_keys(parent);
    *internal_node_num_keys(parent) = original_num_keys + 1;

    if(original_num_keys >= INTERNAL_NODE_MAX_CELLS){
        internal_node_split_insert(table, parent_page_num, child_page_num);
        return;
    }

    uint32_t right_child_page_num = *internal_node_right_child(parent);
    /*An internal node with a right child of INVALID_PAGE_NUM is empty */
    if(right_child_page_num == INVALID_PAGE_NUM){
        *internal_node_right_child(parent)
        return;
    }
    void* right_child = get_page(table->pager, right_child_page_num);

    /*If we already at the max number of cells for a nod, we cannot increment 
    before splitting. Incrementing without inserting a new key/child pair and immediately
    calling internal_node_split_and_insert has the effect of creating a new key at (max_cells+1) with
    and uninitialized value*/

    *internal_node_num_keys(parent) = original_num_keys + 1;


    if(child_max_key > get_node_max_key(table->pager, right_child)){
        /* Replace right child*/
        *internal_node_child(parent, original_num_keys) = right_child_page_num;
        *internal_node_key(parent, original_num_keys) = 
            get_node_max_key(table->pager, right_child);
    } else {
            /* Make room for the new cell */
            for(uint32_t i = original_num_keys; i > index; i--){
                void* destination = internal_node_cell(parent, i);
                void* source = internal_node_cell(parent, i-1);
                memcpy(destination, source, INTERNAL_NODE_CELL_SIZE);
            }
            *internal_node_child(parent, index) = child_page_num;
            *internal_node_key(parent,index) = child_max_key;
    }

}
```
The changes made are important.
We are moving the logic in which increments the parent's number of keys further down in the function definition to ensure that 
this does not happen before the split 
We are also making sure that a child node inserted into an empty internal node wil become that internal node's right_child
without any other operations being performed, since an empty internal node has no keys to manipulate.

When such an internal node is initialized, we initialize its right child with this invalid page number
```c
#define INVALID_PAGE_NUM UINT32_MAX
```

When we initialize an internal node in a the b-tree, we now explicitly set its right child to an invalid page #
    why?
Before, that field wasn’t set at all — it just had whatever random value happened to be in memory.
Sometimes that random value could be 0.
But 0 is also the page number we use for the root of the tree.
That means a brand-new internal node could accidentally think it already had the root as its right child — effectively making it a parent of the root by mistake.
By setting the right child to an invalid page number (a value we know means “nothing is here yet”), we avoid this dangerous confusion.

Also added some safety checks, whenever we try to fetch a child page from an internal node, we now check whether that child's page number is valid
If it's invalid, we throw error

also added guard in function that prints tree, so we don't print a node that doesn't actually have any valid children
```c
if (num_keys > 0) {
        for (uint32_t i = 0; i < num_keys; i++) {
          child = *internal_node_child(node, i);
          print_tree(pager, child, indentation_level + 1);

          indent(indentation_level + 1);
          printf("- key %d\n", *internal_node_key(node, i));
        }
        child = *internal_node_right_child(node);
        print_tree(pager, child, indentation_level + 1);
}
```
```c
void initialize_internal_node(void* node){
    set_node_type(node, NODE_INTERNAL);
    set_node_root(node, false);
    *internal_node_num_keys(node) = 0;
    /*Necessary because the root page number is0; by not initializing an internal node's right child to an invalid page number when initializing the node,
    we may end up with 0 as the node's right child, which makes the node a parent of the root*/
    *internal_node_right_child(node) = INVALID_PAGE_NUM;
}
```


