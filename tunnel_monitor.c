#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <termios.h>
#include <stdbool.h>

// Global variables
const int MAX_CARS = 5; // Threshold for maximum cars in the tunnel
unsigned int cars_in_tunnel = 0;
float smoke_level = 0.0;
float natural_gas_level = 0.0;


bool pause_flag = false;  // Flag to pause the car entering thread
bool alert_flag = false;  // Flag to alert the car entering thread
bool output_flag = true; // Flag to output the current state of the tunnel
char command;


// Mutexes for each variable
pthread_mutex_t cars_mutex;
pthread_mutex_t smoke_mutex;
pthread_mutex_t natural_gas_mutex;
pthread_mutex_t cars_exiting_mutex;
// Cond variable
pthread_cond_t cond;


// Function to set terminal in raw mode to capture single key presses
void set_raw_mode()
{
    struct termios tty; // Terminal I/O API for POSIX
    tcgetattr(STDIN_FILENO, &tty); 
    tty.c_lflag &= ~(ICANON | ECHO); // Disable echo and canonical mode, this way every key press is captured
    tcsetattr(STDIN_FILENO, TCSANOW, &tty); //Change is set immediately
}


// Function to reset terminal to original settings
void reset_terminal_mode()
{
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag |= (ICANON | ECHO); // Enable echo and canonical mode, going back to default
    tcsetattr(STDIN_FILENO, TCSANOW, &tty); //Change is set immediately
}


// Function for the operator to have input for different purposes
void *handle_user_input(void *arg)
{
    set_raw_mode(); //Here we set the input to be capturing single-key presses
    //This was done to bypass the constant output of sensors
    
    
    //Most commands are very self explanatory
    while (1)
    {
        command = getchar();

        if (command == 'p')
        {
            pause_flag = 1;
            printf("\nPausing until further notice\n");
            printf("Please press C to continue...\n");
        }
        else if (command == 'r')
        {
            // Reset logic
            alert_flag = 0;
            cars_in_tunnel = 0;
            smoke_level = 0.0;
            natural_gas_level = 0.0;
            printf("\nResetting all sensor values and state of tunnel \n");
            printf("Please press C to continue...\n");
        }
        else if (command == 'c')
        {
            pause_flag = 0;
            printf("\nContinuing...\n");
        }
        else if (command == 'q')
        {
            printf("\nExiting...\n");
            reset_terminal_mode();
            //We have to reset the terminal before exiting, otherwise the terminal will bug out
            void kill_mutexes();
            exit(0);
        }
        else if (command == 's')
        {
            printf("\nSmoke level: %.1f\n", smoke_level);
        }
        else if (command == 'g')
        {
            printf("\nNatural gas level: %.1f\n", natural_gas_level);
        }
        else if (command == 't')
        {
            printf("\nCars in tunnel: %d\n", cars_in_tunnel);
        }
        else if (command == 'a')
        {
            printf("\nAlert flag: %d\n", alert_flag);
        }
        else if (command == 'o')
        {
            if (output_flag == 0)
            {
                printf("\nTurning on output...\n");
                output_flag = 1;
            }
            else
            {
                printf("\nTurning off output...\n");
                output_flag = 0;
            }
        }
        else if (command == 'e')
        {
            printf("\nCars in tunnel: %d\n", cars_in_tunnel);
            printf("Smoke level: %.1f\n", smoke_level);
            printf("Natural gas level: %.1f\n", natural_gas_level);
            printf("Alert flag: %d\n", alert_flag);
            printf("Pause flag: %d\n", pause_flag);
            printf("Output flag: %d\n", output_flag);
            time_t currentTime;
            time(&currentTime);
            printf("Current time: %s", ctime(&currentTime));
        }
        else if (command == '1')
        {
            smoke_level = 100;
            printf("Setting Smoke level: %.1f\n", smoke_level);
        }
        else if (command == '2')
        {
            natural_gas_level = 100;
            printf("Setting Natural gas level: %.1f\n", natural_gas_level);
        }
        else if (command == '3')
        {
            cars_in_tunnel = 10;
            printf("Setting Cars in tunnel: %d\n", cars_in_tunnel);
        }
        else if (command == '4')
        {
            if(alert_flag==0)
            {
                printf("Toggling alert flag...\n");
                alert_flag = 1;
            }
            else
            {
                printf("Toggling alert flag...\n");
                alert_flag = 0;
            }
        }
        else if (command == 'h')
        {
            printf("\nCommands:\n");
            printf("p - Pause\n");
            printf("r - Reset\n");
            printf("c - Continue\n");
            printf("q - Quit\n");
            printf("s - Print smoke level\n");
            printf("g - Print natural gas level\n");
            printf("t - Print cars in tunnel\n");
            printf("a - Print alert flag\n");
            printf("h - Print commands\n");
            printf("e - Output current state of tunnel\n");
            printf("o - Toggle state of tunnel output\n");
            printf("1 - Set smoke level to 100\n");
            printf("2 - Set natural gas level to 100\n");
            printf("3 - Set cars in tunnel to 10\n");
            printf("4 - Set alert flag to 1 to begin testing\n");
        }
        else if (command == ' ')
        {
            printf("\n");
        }
        else
        {
            printf("\nInvalid command. Please try again.\n");
        }
    }

    reset_terminal_mode();
    //We have to reset the terminal before exiting, otherwise the terminal will bug out
    return NULL;
}


void *measure_smoke(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&smoke_mutex);
        if (pause_flag == 0 && alert_flag == 0)
        {
            smoke_level = (rand() % 1000) / 10.0; // Random CO2 level between 0.0 and 99.9
            if (output_flag == 1)
                printf("Measured smoke level: %.1f\n\n", smoke_level);
        }

        if (smoke_level > 95.0) // If smoke level is above 95, set alert flag
        {
            alert_flag = 1;
        }

        pthread_mutex_unlock(&smoke_mutex);
        sleep(1); // Measure every second so we don't spam the console
    }
    return NULL;
}

void *measure_natural_gas(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&natural_gas_mutex);
        if (pause_flag == 0 && alert_flag == 0)
        {
            natural_gas_level = (rand() % 1000) / 10.0; // Random Natural Gas level between 0.0 and 99.9
            if (output_flag == 1)
                printf("Measured Natural Gas level: %.1f\n", natural_gas_level);
        }

        if (natural_gas_level > 95.0) // If natural gas level is above 95, set alert flag
        {
            alert_flag = 1;
        }

        pthread_mutex_unlock(&natural_gas_mutex);
        sleep(1); // Measure every second so we don't spam the console
    }
    return NULL;
}

void *count_cars_entering(void *arg) // Function to count cars entering the tunnel and alert the operator if necessary
{
    while (1)
    {
        if (alert_flag) // If alert flag is set, alert the operator
        {
            if (smoke_level > 95.0) // If smoke level is above 95, deploy approriate measures
            {
                printf("Smoke detected. Turning sprinkler system on and turning ventilation system to 100%%\n");
                printf("Operator has been alerted...\n");
                while (smoke_level > 0)
                {
                    printf("Smoke level: %.1f\n", smoke_level);
                    smoke_level -= rand() % 30;
                    usleep(500000);
                }
                cars_in_tunnel = 0;
                printf("Sprinkler system turned off. Resuming normal activity\n");
            }
            else if (natural_gas_level > 95.0) // If natural gas level is above 95, deploy approriate measures
            {
                printf("Natural gas detected. Turning ventilation system to 100%%...\n");
                printf("Playing alarm sound...\n");
                while (natural_gas_level > 0)
                {
                    printf("Natural gas level: %.1f\n", natural_gas_level);
                    natural_gas_level -= rand() % 30;
                    usleep(500000);
                }
                printf("Ventilation system adjusted to normal levels. Resuming normal activity\n");
                cars_in_tunnel = 0;
            }
            else // If alert flag is set by operator, deploy approriate measures
            {
                printf("Alert flag set by operator. Turning ventilation system to 100%%...\n");
                printf("Playing alarm sound...\n");
                while (alert_flag)
                {
                    usleep(500000);
                }
            }
            alert_flag = 0;
        }

        pthread_mutex_lock(&cars_mutex); 
        while (cars_in_tunnel >= MAX_CARS)
        {
            printf("Cars entering mutex: Max capacity reached. Pausing entry.\n");
            pthread_cond_wait(&cond, &cars_mutex); // Wait for signal to resume
        }
        if(alert_flag == 0)
            cars_in_tunnel++; //This way we don't increment the cars in tunnel if we are in alert mode
        if (output_flag == 1)
            printf("Car entered. Cars in tunnel: %d\n", cars_in_tunnel);
        pthread_mutex_unlock(&cars_mutex);

        sleep(rand() % 3 + 1); // Time delay between 1 and 4 seconds
        while (pause_flag) // If pause flag is set, wait until it is unset
        {
            sleep(1);
        }
    }

    return NULL;
}

void *count_cars_exiting(void *arg) // Same logic as count_cars_entering
{
    while (1)
    {
        pthread_mutex_lock(&cars_exiting_mutex);
        if (cars_in_tunnel > 0 && alert_flag == 0) // Only decrement if there are cars in the tunnel
        {
            cars_in_tunnel--;
            if (output_flag == 1)
                printf("Car exited. Cars in tunnel: %d\n", cars_in_tunnel);
            pthread_cond_signal(&cond); // Signal the car entering thread
        }
        pthread_mutex_unlock(&cars_exiting_mutex);
        sleep(rand() % 3 + 1); // Time delay between 1 and 4 seconds

        while (pause_flag)
        {
            sleep(1); // If pause flag is set, wait until it is unset
        }
    }
    return NULL;
}

void kill_mutexes() // Function to destroy mutexes and condition variable
{
    pthread_mutex_destroy(&cars_mutex);
    pthread_mutex_destroy(&smoke_mutex);
    pthread_mutex_destroy(&natural_gas_mutex);
    pthread_cond_destroy(&cond);
}

int main()
{
    pthread_t thread1, thread2, co2_thread, natural_gas_thread, user_input_thread;
    time_t currentTime;
    time(&currentTime);
    printf("Current time: %s", ctime(&currentTime));
    // Initialize mutexes and condition variable
    // Also added some error handling for mutexes and threads
    int error;
    error = pthread_mutex_init(&cars_mutex, NULL);
    if (error != 0) {
        fprintf(stderr, "Failed to initialize cars_mutex: %s\n", strerror(error));
        return 1;
    }

    error = pthread_mutex_init(&smoke_mutex, NULL);
    if (error != 0) {
        fprintf(stderr, "Failed to initialize smoke_mutex: %s\n", strerror(error));
        return 1;
    }

    error = pthread_mutex_init(&natural_gas_mutex, NULL);
    if (error != 0) {
        fprintf(stderr, "Failed to initialize natural_gas_mutex: %s\n", strerror(error));
        kill_mutexes();
        return 1;
    }

    error = pthread_cond_init(&cond, NULL);
    if (error != 0) {
        fprintf(stderr, "Failed to initialize cond: %s\n", strerror(error));
        kill_mutexes();
        return 1;
    }

    // Create threads ########################################################################
    error = pthread_create(&thread1, NULL, count_cars_entering, NULL);
    if (error != 0) {
        fprintf(stderr, "Failed to create count_cars_entering thread: %s\n", strerror(error));
        kill_mutexes();
        return 1;
    }

    error = pthread_create(&thread2, NULL, count_cars_exiting, NULL);
    if (error != 0) {
        fprintf(stderr, "Failed to create count_cars_exiting thread: %s\n", strerror(error));
        kill_mutexes();
        return 1;
    }

    error = pthread_create(&co2_thread, NULL, measure_smoke, NULL);
    if (error != 0) {
        fprintf(stderr, "Failed to create measure_smoke thread: %s\n", strerror(error));
        kill_mutexes();
        return 1;
    }

    error = pthread_create(&natural_gas_thread, NULL, measure_natural_gas, NULL);
    if (error != 0) {
        fprintf(stderr, "Failed to create measure_natural_gas thread: %s\n", strerror(error));
        kill_mutexes();
        return 1;
    }

    error = pthread_create(&user_input_thread, NULL, handle_user_input, NULL);
    if (error != 0) {
        fprintf(stderr, "Failed to create handle_user_input thread: %s\n", strerror(error));
        kill_mutexes();
        return 1;
    }

    // Wait for threads to finish ########################################################################
    error = pthread_join(thread1, NULL);
    if (error != 0) {
        fprintf(stderr, "Failed to join count_cars_entering thread: %s\n", strerror(error));
        kill_mutexes();
        return 1;
    }

    error = pthread_join(thread2, NULL);
    if (error != 0) {
        fprintf(stderr, "Failed to join count_cars_exiting thread: %s\n", strerror(error));
        kill_mutexes();
        return 1;
    }

    error = pthread_join(co2_thread, NULL);
    if (error != 0) {
        fprintf(stderr, "Failed to join measure_smoke thread: %s\n", strerror(error));
        kill_mutexes();
        return 1;
    }

    error = pthread_join(natural_gas_thread, NULL);
    if (error != 0) {
        fprintf(stderr, "Failed to join measure_natural_gas thread: %s\n", strerror(error));
        kill_mutexes();
        return 1;
    }

    error = pthread_join(user_input_thread, NULL);
    if (error != 0) {
        fprintf(stderr, "Failed to join handle_user_input thread: %s\n", strerror(error));
        kill_mutexes();
        return 1;
    }

    // Destroy mutexes and condition variable
    void kill_mutexes();
    return 0;
}
