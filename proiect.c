#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

int pause_flag = 0; // Flag to pause the car entering thread
int reset_flag = 0; // Flag to resume the car entering thread

pthread_cond_t cond;

// Global variables
const int MAX_CARS = 10; // Threshold for maximum cars in the tunnel
int cars_in_tunnel = 0;
float co2_level = 0.0;
float natural_gas_level = 0.0;

// Mutexes for each variable
pthread_mutex_t cars_mutex;
pthread_mutex_t co2_mutex;
pthread_mutex_t natural_gas_mutex;
pthread_mutex_t cars_exiting_mutex;
pthread_mutex_t flag_mutex;

void* handle_user_input(void* arg) {
    char command[100];
    while (1) {
        printf("Enter command (Pause, Reset, Resume): ");
        fflush(stdout);  // Flush the output buffer

        scanf("%99s", command);
        while (getchar() != '\n');  // Clear the input buffer

        pthread_mutex_lock(&flag_mutex);
        if (strcmp(command, "Pause") == 0) {
            pause_flag = 1;
        } else if (strcmp(command, "Reset") == 0) {
            cars_in_tunnel = 0;
            co2_level = 0.0;
            natural_gas_level = 0.0;
        } else if (strcmp(command, "Resume") == 0) {
            pause_flag = 0;
        }
        pthread_mutex_unlock(&flag_mutex);
    }
    return NULL;
}


void* measure_co2(void* arg) {
    while (1) {
        pthread_mutex_lock(&co2_mutex);
        co2_level = (rand() % 1000) / 10.0; // Random CO2 level between 0.0 and 99.9
        printf("Measured CO2 level: %.1f\n", co2_level);

        if (co2_level > 80.0) {
            pthread_mutex_lock(&flag_mutex);
            pause_flag = 1;
            pthread_mutex_unlock(&flag_mutex);
        }

        pthread_mutex_unlock(&co2_mutex);
        sleep(1); // Measure every second
    }
    return NULL;
}

void* measure_natural_gas(void* arg) {
    while (1) {
        pthread_mutex_lock(&natural_gas_mutex);
        natural_gas_level = (rand() % 1000) / 10.0; // Random Natural Gas level between 0.0 and 99.9
        printf("Measured Natural Gas level: %.1f\n", natural_gas_level);

        if (natural_gas_level > 80.0) {
            pthread_mutex_lock(&flag_mutex);
            pause_flag = 1;
            pthread_mutex_unlock(&flag_mutex);
        }

        pthread_mutex_unlock(&natural_gas_mutex);
        sleep(1); // Measure every second
    }
    return NULL;
}


void* count_cars_entering(void* arg) {
    while (1) {
        pthread_mutex_lock(&flag_mutex);
        if (pause_flag) {
            pthread_mutex_unlock(&flag_mutex);
            printf("High CO2/Natural Gas levels. Pausing entry for 10 seconds.\n");
            sleep(10); // Sleep for 10 seconds
            pthread_mutex_lock(&flag_mutex);
            pause_flag = 0; // Reset flag
        }
        pthread_mutex_unlock(&flag_mutex);

        pthread_mutex_lock(&cars_mutex);
        while (cars_in_tunnel >= MAX_CARS) {
            printf("Cars entering mutex: Max capacity reached. Pausing entry.\n");
            pthread_cond_wait(&cond, &cars_mutex); // Wait for signal to resume
        }
        cars_in_tunnel++;
        printf("Car entered. Cars in tunnel: %d\n", cars_in_tunnel);
        pthread_mutex_unlock(&cars_mutex);

        sleep(rand() % 3); // Time delay between 0 and 2 seconds
    }
    return NULL;
}

void* count_cars_exiting(void* arg) {
    while (1) {
        pthread_mutex_lock(&cars_exiting_mutex);
        if (cars_in_tunnel > 0) {
            cars_in_tunnel--;
            printf("Car exited. Cars in tunnel: %d\n", cars_in_tunnel);
            pthread_cond_signal(&cond); // Signal the car entering thread
        }
        pthread_mutex_unlock(&cars_exiting_mutex);
        sleep(rand()%6); //Time delay between 0 and 9 seconds

                int should_pause = pause_flag;
        pthread_mutex_unlock(&flag_mutex);

        if (should_pause) {
            sleep(1); // Or use a condition variable to sleep until resumed
            continue;
        }
    }
    return NULL;
}
void* monitor_tunnel(void* arg) {
    while (1) {
        pthread_mutex_lock(&cars_mutex);
        if (cars_in_tunnel >= MAX_CARS) {
            printf("Monitor: Max capacity reached. Pausing entry until there is space.\n");
            while (cars_in_tunnel >= MAX_CARS) {
                // Wait for the condition to change (car exits)
                pthread_cond_wait(&cond, &cars_mutex);
            }
            // Now there is space, signal the car entering thread to resume
            pthread_cond_signal(&cond);
        }
        pthread_mutex_unlock(&cars_mutex);
        usleep(100000); // Check periodically

                int should_pause = pause_flag;
        pthread_mutex_unlock(&flag_mutex);

        if (should_pause) {
            sleep(1); // Or use a condition variable to sleep until resumed
            continue;
        }
    }
    return NULL;
}


int main() {
    pthread_t thread1, thread2, monitor_thread, co2_thread, natural_gas_thread, user_input_thread;

    // Initialize mutexes and condition variable
    pthread_mutex_init(&cars_mutex, NULL);
    pthread_mutex_init(&co2_mutex, NULL);
    pthread_mutex_init(&natural_gas_mutex, NULL);
    pthread_mutex_init(&flag_mutex, NULL); // Make sure this is initialized
    pthread_cond_init(&cond, NULL);

    // Create threads
    pthread_create(&thread1, NULL, count_cars_entering, NULL);
    pthread_create(&thread2, NULL, count_cars_exiting, NULL);
    pthread_create(&monitor_thread, NULL, monitor_tunnel, NULL);
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
    pthread_mutex_destroy(&flag_mutex);
    pthread_cond_destroy(&cond);

    return 0;
}
