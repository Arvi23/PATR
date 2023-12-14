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
float co2_level = 0.0;
float natural_gas_level = 0.0;
int pause_flag = 0; // Flag to pause the car entering thread
int alert_flag = 0; // Flag to alert the car entering thread
// Mutexes for each variable
pthread_mutex_t cars_mutex;
pthread_mutex_t co2_mutex;
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
            co2_level = 0.0;
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
        else
        {
            printf("\nInvalid command. Please try again.\n");
        }
    }

    reset_terminal_mode();
    return NULL;
}

void *measure_co2(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&co2_mutex);
        if (pause_flag == 0 && alert_flag == 0)
        {
            co2_level = (rand() % 1000) / 10.0; // Random CO2 level between 0.0 and 99.9
            printf("Measured CO2 level: %.1f\n", co2_level);
        }

        if (co2_level > 99.0)
        {
            alert_flag = 1;
        }

        pthread_mutex_unlock(&co2_mutex);
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
        if (pause_flag == 0 && alert_flag == 0)
        {
            natural_gas_level = (rand() % 1000) / 10.0; // Random Natural Gas level between 0.0 and 99.9
            printf("Measured Natural Gas level: %.1f\n", natural_gas_level);
        }

        if (natural_gas_level > 99.0)
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
            printf("High CO2/Natural Gas levels. Pausing entry \n");
            printf("Evacuating cars...\n");
            while (cars_in_tunnel)
            {
                sleep(1);
            }
            co2_level = 0.0;
            natural_gas_level = 0.0;
            printf("Cars evacuated. Resuming entry...\n");
            alert_flag = 0;
        }

        pthread_mutex_lock(&cars_mutex);
        while (cars_in_tunnel >= MAX_CARS)
        {
            printf("Cars entering mutex: Max capacity reached. Pausing entry.\n");
            pthread_cond_wait(&cond, &cars_mutex); // Wait for signal to resume
        }
        cars_in_tunnel++;
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
        if (cars_in_tunnel > 0)
        {
            cars_in_tunnel--;
            printf("Car exited. Cars in tunnel: %d\n", cars_in_tunnel);
            pthread_cond_signal(&cond); // Signal the car entering thread
        }
        pthread_mutex_unlock(&cars_exiting_mutex);
        sleep(rand() % 6); // Time delay between 0 and 9 seconds

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
    pthread_mutex_init(&co2_mutex, NULL);
    pthread_mutex_init(&natural_gas_mutex, NULL);
    pthread_cond_init(&cond, NULL);

    // Create threads
    pthread_create(&thread1, NULL, count_cars_entering, NULL);
    pthread_create(&thread2, NULL, count_cars_exiting, NULL);
    pthread_create(&co2_thread, NULL, measure_co2, NULL);
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
    pthread_mutex_destroy(&co2_mutex);
    pthread_mutex_destroy(&natural_gas_mutex);
    pthread_cond_destroy(&cond);

    return 0;
}
