#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "interrupts.h"
#include "rtc_driver.h"
#include "filesystem_driver.h"
#include "terminal.h"
#include "syscalls.h"
#define PASS 1
#define FAIL 0

#define NUM_BYTES 128 // Set this value to set nbytes parameter for system calls.

/* format these macros as you see fit */
#define TEST_HEADER 	\
    printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__);
#define TEST_OUTPUT(name, result) \
    printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure() {
    /* Use exception #15 for assertions, otherwise
       reserved by Intel */
    asm volatile("int $15");
}

/* Checkpoint 1 tests */

/* IDT Test - Example
 *
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test()
{
    TEST_HEADER;

    int i;
    int result = PASS;
    for (i = 0; i < 10; ++i){
        if ((idt[i].offset_15_00 == NULL) &&
                (idt[i].offset_31_16 == NULL)){
            assertion_failure();
            result = FAIL;
        }
    }

    return result;
}

/* IDT Test - Divide by zero
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Div by zero exception
 * Files: interrupt.c
 */
int div_zero_test()
{
    TEST_HEADER;

    int i;
    int result = PASS;
    int j;
    j = 0;
    i = 5/j;
    return result;
}


/* IDT Test - Deref Null
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Dereferencing a null pointer
 * Files: interrupt.c, paging.S
 */
int dref_null_test()
{
    TEST_HEADER;

    int i;
    int result = PASS;
    int * p;
    p = NULL;
    i = *(p);
    return result;
}
// add more tests here
    /*Print out the next 100 RTC interrupts*/

/* Checkpoint 2 tests */

/* System Call Test - Write
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: calls the write system call for file directories
 * Files: syscalls.c
 */
int syscall_write_fd_test()
{
    TEST_HEADER;

    uint8_t buffer[200]; //buffer used for system call
    int fdnum; //used to hold fd number
    int ret; //used to get return val from syscall

    //opens file and tries to write
    fdnum = do_call(SYS_OPEN,(int)"frame0.txt",0,0);
    if (fdnum == -1){return FAIL;}
    //write will always return -1
    ret = do_call(SYS_WRITE, fdnum, (int)buffer, 200);
    if(ret != -1){return FAIL;}
    do_call(SYS_CLOSE, fdnum, 0, 0);

    //opens directroy and tries to write
    fdnum = do_call(SYS_OPEN,(int)".",0,0);
    if (fdnum == -1){return FAIL;}
    ret = do_call(SYS_WRITE, fdnum, (int)buffer, 200);
    //write will always return -1
    if(ret != -1){return FAIL;}
    do_call(SYS_CLOSE, fdnum, 0, 0);

    return PASS;
}


/* System Call Test - read file
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: calls the read system call for file
 * Files: syscalls.c
 */
int syscall_read_file_test()
{

    TEST_HEADER;

    int result; //used to get return val from syscall
    int fdnum; //used to hold fd number
    uint8_t buffer[200]; //buffer used for system call

    //opens file and tries to read from file
    fdnum = do_call(SYS_OPEN,(int)"frame0.txt",0,0);
    if (fdnum == -1){return FAIL;}
    //reads from file and prints a fish
    result = do_call(SYS_READ, fdnum, (int)buffer, 10000);
    terminal_write(0,(const void*) buffer, (int32_t)strlen((const int8_t* )buffer));
    if(result == -1){return FAIL;}
    do_call(SYS_CLOSE, fdnum, 0, 0);

    return PASS;
  }

  /* System Call Test - read directory
   * Inputs: None
   * Outputs: PASS/FAIL
   * Side Effects: None
   * Coverage: calls the read system call for file
   * Files: syscalls.c
   */
  int syscall_read_directory_test(){

    //Reads file directory information
    int fdnum; //used to hold fd number
    uint8_t buffer2[700]; //buffer used for system call
    int i; //loop variable used for iteration
    int result; //used to get return val from syscall

    //opens a directory and tries to read
    fdnum = do_call(SYS_OPEN,(int)".",0,0);
    if (fdnum == -1){return FAIL;}
    i = 0;
    //print out contents of directory, when directory empty, it returns 0
    for(; i < 20; i++){
      result = do_call(SYS_READ, fdnum, (int)(buffer2 + strlen((const int8_t* )buffer2)), 10000);
      buffer2[strlen((const int8_t* )buffer2)] = '\n';
      if(result == -1){return FAIL;}
    }
    terminal_write(0, buffer2, strlen((const int8_t* )buffer2));
    do_call(SYS_CLOSE, fdnum, 0, 0);

    return PASS;
}

/* System Call Test - Read and Write rtc
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: calls the read and write system call for rtc
 * Files: syscalls.c
 */
int syscall_read_write_rtc_test(){
    TEST_HEADER;
    int i; //loop variable used for iteration
    int ret; //used to get return val from syscall
    int fdnum; //used to hold fd number
    char c = 'i'; //char used for print
    char * ptr = & c; // char used for print

    terminal_clear();
    fdnum = do_call(SYS_OPEN,(int)"rtc",0,0);
    /** Set the rate to 15 */
    ret = do_call(SYS_WRITE,fdnum,0,2);


    /** Print out the next 10 RTC interrupts */
    for (i = 0; i < 10; i++) {
        do_call(SYS_READ,fdnum,0,0);
        terminal_write('\0', ptr, 1);
    }

    terminal_clear();

    /** Set the rate to 32 */
    do_call(SYS_WRITE,fdnum,0,32);

    /** Print out the next 100 RTC interrupts */
    for (i = 0; i < 100; i++) {
        do_call(SYS_READ,fdnum,0,0);
        terminal_write('\0', ptr, 1);

    }

    terminal_clear();

    /** Set the rate to 512 */
    do_call(SYS_WRITE,fdnum,0,512);

    /** Print out the next 1000 RTC interrupts */
    for (i = 0; i < 1000; i++) {
        do_call(SYS_READ,fdnum,0,0);
        terminal_write('\0', ptr, 1);
    }
    terminal_clear();
    //tries to write an incorrect amount
    ret = do_call(SYS_WRITE,fdnum,0,420);
    if(ret != -1){ return FAIL;}
    do_call(SYS_CLOSE,fdnum,0,0);

  return PASS;
}

/* System Call Test - close
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: calls the close system call
 * Files: syscalls.c
 */
int syscall_close_test()
{
    // TODO: Write me.
    TEST_HEADER;
    int close; //return value from closing
    int fdnum; //used to hold fd number
    int second; //return value from second file

    //checks that file opens and closes correctly
    fdnum = do_call(SYS_OPEN,(int)"frame0.txt",0,0);
    if (fdnum == -1){return FAIL;}
    close = do_call(SYS_CLOSE, fdnum, 0, 0);
    if(close == -1){return FAIL;}
    //check that next file is placed in correct space
    second = do_call(SYS_OPEN,(int)"frame1.txt",0,0);
    if (second == -1){return FAIL;}
    if (second != fdnum){return FAIL;}
    do_call(SYS_CLOSE, second, 0, 0);

    //checks that directory opens and closes correctly
    fdnum = do_call(SYS_OPEN,(int)".",0,0);
    if (fdnum == -1){ return FAIL;}
    close = do_call(SYS_CLOSE, fdnum, 0, 0);
    if(close == -1){return FAIL;}

    //checks that rtc opens and closes correctly
    fdnum = do_call(SYS_OPEN,(int)"rtc",0,0);
    if (fdnum == -1){ return FAIL;}
    close = do_call(SYS_CLOSE, fdnum, 0, 0);
    if(close == -1){return FAIL;}

    return PASS;
}

/* System Call Test - open
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: calls the open system call
 * Files: syscalls.c
 */
int syscall_open_test()
{
    TEST_HEADER;
    int fdnum; //used to hold fd number
    int ret;  //used to get return val from syscall

    //opens a file
    fdnum = do_call(SYS_OPEN, (int)"frame0.txt", 0, 0);
    if (fdnum == -1) {return FAIL;}
    //tries opening again, should be invalid
    ret = do_call(SYS_OPEN, (int)"frame0.txt", 0, 0);
    if (ret != -1) {return FAIL;}
    do_call(SYS_CLOSE, fdnum, 0, 0);

    //opens a directory
    fdnum = do_call(SYS_OPEN, (int)".", 0, 0);
    if (fdnum == -1) {return FAIL;}
    //tries opening again, should be invalid
    ret = do_call(SYS_OPEN,(int)".",0,0);
    if(ret != -1){return FAIL;}
    do_call(SYS_CLOSE, fdnum, 0, 0);

    //opens rtc
    fdnum = do_call(SYS_OPEN,(int)"rtc",0,0);
    if (fdnum == -1){return FAIL;}
    //tries opening again, should be invalid
    ret = do_call(SYS_OPEN,(int)"rtc",0,0);
    if(ret != -1){return FAIL;}
    do_call(SYS_CLOSE, fdnum, 0, 0);

    return PASS;
}



/* RTC Test - Write
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: changing the frequency of the RTC
 * Files: rtc_drivers.c
 */

int rtc_write_test()
{
	TEST_HEADER;

    int result = PASS;
    int i = 0;
    terminal_clear();

    /** Set the rate to 15 */
    rtc_write(15,0,0);

    /** Print out the next 10 RTC interrupts */
    for (i = 0; i < 10; i++) {
        rtc_read(0,0,0);
    }

    terminal_clear();

    /** Set the rate to 10 */
    rtc_write(10,0,0);

    /** Print out the next 100 RTC interrupts */
    for (i = 0; i < 100; i++) {
        rtc_read(0,0,0);
    }

    terminal_clear();

    /** Set the rate to 5 */
    rtc_write(5,0,0);

    /** Print out the next 1000 RTC interrupts */
    for (i = 0; i < 1000; i++) {
        rtc_read(0,0,0);
    }

    return result;
}

/* RTC Test - Open
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Sets the frequency of the rtc to two a second
 * Files: rtc_drivers.c
 */

int rtc_open_test()
{
	TEST_HEADER;

    int result = PASS;
    int i = 0;
    terminal_clear();

    /** open the RTC: sets the rate to twice a second */
    // TODO: Fix parameters.
    rtc_open(0);

    /** Print the next 50 interrupts */
    for (i = 0; i < 50; i++) {
        rtc_read(0,0,0);
    }

    terminal_clear();

    return result;
}

/* Terminal Driver - Open test
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: terminal driver open system call
 * Files: terminal.c
 */
int test_terminal_open()
{
    // Test the terminal open clears the read buffer.
    TEST_HEADER;

    int result = PASS;

    int val = terminal_open('\0');
    if (val != SUCCESS) {
        result = FAIL;
    }

    // next_line();

    return result;
}

/* Terminal Driver - Write test
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: terminal driver write system call
 * Files: terminal.c
 */
int test_terminal_write()
{
    // Test if the first paragraph of the Declaration of the united States of America
    // writes to the terminal.
    TEST_HEADER;

    int result = PASS;

    char string[475] =
        "The unanimous Declaration of the thirteen united States of America, "
        "When in the Course of human events, it becomes necessary for one people "
        "to dissolve the political bands which have connected them with another, "
        "and to assume among the powers of the earth, the separate and equal "
        "station to which the Laws of Nature and of Nature's God entitle them, "
        "a decent respect to the opinions of mankind requires that they should "
        "declare the causes which impel them to the separation.\0";

    int val = terminal_write(1, string, 475);
    if (val != 475) {
        result = FAIL;
    }

    next_line();

    return result;
}

/* Terminal Driver - Read Test
 * Inputs: Keyboard input.
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Both terminal and keyboard driver system calls.
 * Files: terminal.c, keyboard.c
 */
int test_terminal_read()
{
    // Test if the terminal reads the keyboard buffer
    // after pressing enter.
    TEST_HEADER;

    printf("Hi! What's your name? ");
    cursor_update();

    int result = PASS;

    int num_bytes = NUM_BYTES;
    unsigned char string[MAX_BUFFER_SIZE];

    int val = terminal_read(1, string, num_bytes);

    if (num_bytes < 1) {
        if (val != FAILURE) {
            result = FAIL;
        } else {
            return PASS;
        }
    }

    int i;
    for (i = 0; i < MAX_BUFFER_SIZE; i++) {
        if (string[i] == '\n') {
            string[i] = '\0';
            break;
        }
    }

    printf("Hello, ");
    printf((char *) string);
    printf(".");

    next_line();

    if ((val != num_bytes) && (strlen((char *) string) != num_bytes)) {
        result = FAIL;
    }

    return result;
}

/* Keyboard Driver - Open/Read test
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: keyboard driver open & read system calls
 * Files: terminal.c
 */
int test_keyboard_open()
{
    // Test the keyboard open clears the keyboard buffer.
    TEST_HEADER;

    int result = PASS;

    int val = keyboard_open('\0');
    if (val != SUCCESS) {
        result = FAIL;
    }

    char * string[MAX_BUFFER_SIZE];
    val = keyboard_read(0, string, 1);

    if (val != 0 || string[0] != '\0') {
        result = FAIL;
    }

    // next_line();

    return result;
}

/* Keyboard Driver - Write Test
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Keyboard Driver Write system call.
 * Files: keyboard.c
 */
int test_keyboard_write()
{
    // Tests if keyboard write will return a failure.
    TEST_HEADER;

    int result = PASS;

    int retval = keyboard_write(0, "TESTING", 7);

    if (retval != FAILURE) {
        result = FAIL;
    }

    return result;
}

/* File Test - Read Text
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Reading a .txt file
 * Files: filesystem_driver.c
 */
int file_read_text_test()
{
    TEST_HEADER;

    int result = PASS;
    uint8_t buf[10000];

    /** Open a simple text file */
    int fd = file_open((const uint8_t *)"frame0.txt");

    int length = 10000;

    /** Read the text file */
    file_read(fd,buf,length);

    /** Print the file to the terminal */
    terminal_write(0,buf,length);

    /** Close the file */
    file_close(fd);
    next_line();
    return result;
}

/* File Test - Read Binary
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Reading a binary file
 * Files: filesystem_driver.c
 */
int file_read_binary_test()
{
    TEST_HEADER;

    int result = PASS;
    uint8_t buf[10000];

    /** Open an executable file */
    int fd = file_open((const uint8_t *)"hello");
    int length = 10000;

    /** Read the file */
    file_read(fd,buf,length);

    /** Print the file to the screen */
    terminal_write(0,buf,length);

    /** Close the file */
    file_close(fd);
    next_line();
    return result;
}

/* File Test - Read Long
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Reading a long file
 * Files: filesystem_driver.c
 */
int file_read_long_test()
{
    TEST_HEADER;

    int result = PASS;
    uint8_t buf[40000];

    /** Open a long file */
    int fd = file_open((const uint8_t *)"verylargetextwithverylongname.txt");
    int length = 40000;

    /** Read the file into a buffer */
    file_read(fd,buf,length);

    /** Write the file to the terminal */
    terminal_write(0,buf,length);

    /** Close the file */
    file_close(fd);
    next_line();
    return result;
}

/* File Test - Close
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Closing files
 * Files: filesystem_driver.c
 */
int file_close_test()
{
    TEST_HEADER;

    int result = PASS;
    int close_ret = 0;
    /** Open a long file */
    int fd = file_open((const uint8_t *)"verylargetextwithverylongname.tx");

    /** Close the file */
    close_ret = file_close(fd);

    if (close_ret == -1) {
        printf("Unable to close file\n");
        result = FAIL;
    }

    /** Attempt to close an already closed file */
    close_ret = file_close(fd);

    if (close_ret != -1) {
        printf("Shouldn't be able to close a closed file\n");
        result = FAIL;
    }
    // next_line();
    return result;
}

/* Dir Test - Close
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Closing dirs
 * Files: filesystem_driver.c
 */
int dir_close_test() {
    TEST_HEADER;

    int result = PASS;
    int close_ret = 0;

    /** Opens a dir */
    int fd = directory_open((const uint8_t *)".");

    /** Close the dir */
    close_ret = directory_close(fd);

    if (close_ret != -1) {
        printf("Unable to close dir\n");
        result = FAIL;
    }

    /** Attempt to close an already closed dir */
    close_ret = directory_close(fd);

    if (close_ret != -1) {
        printf("Shouldn't be able to close a closed file\n");
        result = FAIL;
    }

    next_line();
    return result;
}


/* Dir Test - read
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Reading a dir file
 * Files: filesystem_driver.c
 */
int dir_read_test()
{
    TEST_HEADER;
    uint8_t buf[2000];
    int result = PASS;


    /** Open a directory */
    int fd = directory_open((const uint8_t *) "frame0.txt");
    clear();

    /** Read the directory */
    int i = 0;
    for(; i < 400; i++){
    directory_read(fd, buf + strlen((const int8_t*)buf), 500);

  }
  terminal_write(0,buf,500);

    /** Close the file */
    directory_close(fd);
    next_line();
    return result;
}


/* Dir Test - Open
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Opening a dir
 * Files: filesystem_driver.c
 */
int dir_open_test()
{
    TEST_HEADER;
    int result = PASS;
    int fd = 0;
    int fd_2 = 0;

    /** Check that opening a good dir succeeds */
    fd = directory_open((const uint8_t *)"frame0.txt");
    if (fd == -1) {
        printf("Unable to open dir\n");
        result = FAIL;
    }
    directory_close(fd);

    /** Check that opening a the same dir twice fails */
    fd = directory_open( (const uint8_t *)"frame0.txt");
    if (fd_2 == -1) {
        printf("Unable to open a dir twice\n");
        result = FAIL;
    }
    directory_close(fd);

    return result;
}
/* File Test - Open
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Opening a regular file
 * Files: filesystem_driver.c
 */
int file_open_test()
{
    TEST_HEADER;
    int result = PASS;
    int fd = 0;
    int fd_2 = 0;

    /** Check that opening a good file succeeds */
    fd = file_open((const uint8_t *)"frame0.txt");
    if (fd == -1) {
        printf("Unable to open file\n");
        result = FAIL;
    }
    file_close(fd);

    /** Check that opening a bad file fails */
    fd = file_open((const uint8_t *)"tiger.txt");
    if (fd != -1) {

        printf("Not rejecting bad filename\n");
        result = FAIL;
    }
    file_close(fd);

    /** Check that opening a the same file twice fails */
    fd = file_open( (const uint8_t *)"frame0.txt");
    if (fd_2 == -1) {
        printf("Unable to open file\n");
        result = FAIL;
    }
    file_close(fd);

    return result;
}

/* File Test - Write
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Writing to a regular file
 * Files: filesystem_driver.c
 */
int file_write_test()
{
    TEST_HEADER;
    int result = PASS;
    int fd = 0;
    int ret = 0;

    /** Open a file */
    fd = file_open((const uint8_t *)"frame0.txt");

    /** Attempt to write to the file */
    ret = file_write(fd,0,0);
    if (ret != -1) {
        printf("Shouldn't be able to write to regular file\n");
        result = FAIL;
    }

    /** Close the file */
    file_close(fd);

    return result;
}

/* Dir Test - Write
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Writing to a directory
 * Files: filesystem_driver.c
 */
int dir_write_test()
{
    TEST_HEADER;
    int result = PASS;
    int fd = 0;
    int ret = 0;

    /** Open a directory */
    fd = directory_open((const uint8_t *)"frame0.txt");

    /** Attempt to write to the directory */
    ret = directory_write(fd,0,0);
    if (ret != -1) {
        printf("Shouldn't be able to write to regular file\n");
        result = FAIL;
    }

    /** Close the directory */
    directory_close(fd);

    return result;
}

/* Checkpoint 3 tests */

/* System Call Test - Open
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: calls the write system call
 * Files: interrupt.c
 */


/* System Call Test - Execute
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: executes the shell program
 * Files: interrupt.c
 */
int syscall_exe_test()
{
    // TODO: Write me.
    TEST_HEADER;
    do_call(2,(int)"shell",0,0);

    return 1;
}

/* Syscall to stdout write
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: executes the shell program
 * Files: interrupt.c, syscalls.c,
 *      syscalls_wrapper.S, terminal.c
 */
int syscall_terminal_write_test()
{
    // TODO: Write me.
    TEST_HEADER;
    uint8_t test[4];
    test[0] = 'T';
    test[1] = 'E';
    test[2] = 'S';
    test[3] = '\n';
    do_call(SYS_WRITE,1,(int)test,4);

    return 1;
}

/* System Call Test - Execute
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: executes the shell program
 * Files: syscalls.c, syscalls_wrapper.S
 */
int syscall_parameter_test()
{
    TEST_HEADER;
    int retval = do_call(0, 0, 0, 0);
    // printf("Test 1: %d \n", retval);
    if (retval >= 0) {
        return FAIL;
    }

    retval = do_call(11, 0, 0, 0);
    // printf("Test 1: %d \n", retval);
    if (retval >= 0) {
        return FAIL;
    }

    return PASS;
}

/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests()
{
    // Set screen position to top left corner.
    terminal_clear();

    // launch your tests here
    // TEST_OUTPUT("idt_test", idt_test());
    // TEST_OUTPUT("divide by zero test", div_zero_test());
    // TEST_OUTPUT("deref null test", dref_null_test());

    // TEST_OUTPUT("Test Syscall: Open" , syscall_open_test());
    // TEST_OUTPUT("Test Syscall: Close", syscall_close_test());
    // TEST_OUTPUT("Test Syscall: Read File" , syscall_read_file_test());
    // TEST_OUTPUT("Test Syscall: Read Directory" , syscall_read_directory_test());
    // TEST_OUTPUT("Test Syscall: Read and Write RTC" , syscall_read_write_rtc_test());
    // TEST_OUTPUT("Test Syscall: Write", syscall_write_fd_test());

    // TEST_OUTPUT("Test File: Open", file_open_test());
    // TEST_OUTPUT("Test File: Read Binary", file_read_binary_test());
    // TEST_OUTPUT("Test File: Read Text", file_read_text_test());
    // TEST_OUTPUT("Test File: Read Long", file_read_long_test());
    // TEST_OUTPUT("Test File: Close", file_close_test());
    // TEST_OUTPUT("Test File: Write", file_write_test());

    // TEST_OUTPUT("Test Dir: Read", dir_read_test());
    // TEST_OUTPUT("Test Dir: write", dir_write_test());
    // TEST_OUTPUT("Test Dir: open", dir_open_test());
    // TEST_OUTPUT("Test Dir: Close", dir_close_test());

    // TEST_OUTPUT("keyboard_open", test_keyboard_open());
    // TEST_OUTPUT("keyboard_write", test_keyboard_write());

    // TEST_OUTPUT("terminal_open", test_terminal_open());
    // TEST_OUTPUT("terminal_write", test_terminal_write());
    // TEST_OUTPUT("terminal_read", test_terminal_read());

    // TEST_OUTPUT("Test RTC: Open", rtc_open_test());
    // TEST_OUTPUT("Test RTC: Write", rtc_write_test());

    // TEST_OUTPUT("Test File: syscall_open", syscall_terminal_write_test());

    // TEST_OUTPUT("Test File: system_call", syscall_parameter_test());

     TEST_OUTPUT("Test File: syscall_execute" , syscall_exe_test());
    //cursor_update();
}
