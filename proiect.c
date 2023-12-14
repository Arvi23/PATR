#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <termios.h>
// Cond variable
pthread_cond_t cond;

// Global variables
const int MAX_CARS = 5; // Threshold for maximum cars in the tunnel
int cars_in_tunnel = 0;
float smoke_level = 0.0;
float natural_gas_level = 0.0;
int pause_flag = 0;  // Flag to pause the car entering thread
int alert_flag = 0;  // Flag to alert the car entering thread
int output_flag = 1; // Flag to output the current state of the tunnel
// Mutexes for each variable
pthread_mutex_t cars_mutex;
pthread_mutex_t smoke_mutex;
pthread_mutex_t natural_gas_mutex;
pthread_mutex_t cars_exiting_mutex;
char command;

// Function to set terminal in raw mode to capture single key presses
void set_raw_mode()
{
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag &= ~(ICANON | ECHO); // Disable echo and canonical mode
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

// Function to reset terminal to original settings
void reset_terminal_mode()
{
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag |= (ICANON | ECHO); // Enable echo and canonical mode
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

void *handle_user_input(void *arg)
{
    set_raw_mode();

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
        }
        else if (command == '1')
        {
            smoke_level = 100;
            printf("Smoke level: %.1f\n", smoke_level);
        }
        else if (command == '2')
        {
            natural_gas_level = 100;
            printf("Natural gas level: %.1f\n", natural_gas_level);
        }
        else if (command == '3')
        {
            cars_in_tunnel = 10;
            printf("Cars in tunnel: %d\n", cars_in_tunnel);
        }
        else if (command == '4')
        {
            alert_flag = 1;
            printf("Alert flag: %d\n", alert_flag);
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
                printf("Measured smoke level: %.1f\n", smoke_level);
        }

        if (smoke_level > 95.0)
        {
            alert_flag = 1;
        }

        pthread_mutex_unlock(&smoke_mutex);
        sleep(1); // Measure every second
    }
    while (pause_flag)
    {
        sleep(1);
    }
    return NULL;
}

void *measure_natural_gas(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&natural_gas_mutex);
        if (pause_flag == 0 && alert_flag == 0 && output_flag == 1)
        {
            natural_gas_level = (rand() % 1000) / 10.0; // Random Natural Gas level between 0.0 and 99.9
            printf("Measured Natural Gas level: %.1f\n", natural_gas_level);
        }

        if (natural_gas_level > 95.0)
        {
            alert_flag = 1;
        }

        pthread_mutex_unlock(&natural_gas_mutex);
        sleep(1); // Measure every second
    }
    while (pause_flag)
    {
        sleep(1);
    }
    return NULL;
}

void *count_cars_entering(void *arg)
{
    while (1)
    {
        if (alert_flag)
        {
            if (smoke_level > 95.0)
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
            if (natural_gas_level > 95.0)
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

            alert_flag = 0;
        }

        pthread_mutex_lock(&cars_mutex);
        while (cars_in_tunnel >= MAX_CARS)
        {
            printf("Cars entering mutex: Max capacity reached. Pausing entry.\n");
            pthread_cond_wait(&cond, &cars_mutex); // Wait for signal to resume
        }
        cars_in_tunnel++;
        if (output_flag == 1)
            printf("Car entered. Cars in tunnel: %d\n", cars_in_tunnel);
        pthread_mutex_unlock(&cars_mutex);

        sleep(rand() % 3); // Time delay between 0 and 2 seconds
        while (pause_flag)
        {
            sleep(1);
        }
    }

    return NULL;
}

void *count_cars_exiting(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&cars_exiting_mutex);
        if (cars_in_tunnel > 0 && alert_flag == 0)
        {
            cars_in_tunnel--;
            if (output_flag == 1)
                printf("Car exited. Cars in tunnel: %d\n", cars_in_tunnel);
            pthread_cond_signal(&cond); // Signal the car entering thread
        }
        pthread_mutex_unlock(&cars_exiting_mutex);
        sleep(rand() % 3); // Time delay between 0 and 3 seconds

        while (pause_flag)
        {
            sleep(1);
        }
    }
    return NULL;
}

int main()
{
    pthread_t thread1, thread2, monitor_thread, co2_thread, natural_gas_thread, user_input_thread;

    // Initialize mutexes and condition variable
    pthread_mutex_init(&cars_mutex, NULL);
    pthread_mutex_init(&smoke_mutex, NULL);
    pthread_mutex_init(&natural_gas_mutex, NULL);
    pthread_cond_init(&cond, NULL);

    // Create threads
    pthread_create(&thread1, NULL, count_cars_entering, NULL);
    pthread_create(&thread2, NULL, count_cars_exiting, NULL);
    pthread_create(&co2_thread, NULL, measure_smoke, NULL);
    pthread_create(&natural_gas_thread, NULL, measure_natural_gas, NULL);
    pthread_create(&user_input_thread, NULL, handle_user_input, NULL); // Create user input thread

    // Wait for threads to finish
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(monitor_thread, NULL);
    pthread_join(co2_thread, NULL);
    pthread_join(natural_gas_thread, NULL);
    pthread_join(user_input_thread, NULL); // Wait for user input thread

    // Destroy mutexes and condition variable
    pthread_mutex_destroy(&cars_mutex);
    pthread_mutex_destroy(&smoke_mutex);
    pthread_mutex_destroy(&natural_gas_mutex);
    pthread_cond_destroy(&cond);

    return 0;
}
