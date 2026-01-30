/**
 * main.cpp
 * Entry point. Initializes threads and provides the CLI Menu.
 * OS SIMULATOR BY ARSAL NAVEED F2023266900
 */

#include "os_sim.h"
#include <thread>
#include <chrono>

// Define Shared Variables
vector<int> available_resources;
queue<Process> buffer;
vector<Process> ready_queue;
vector<Process> blocked_queue;
sem_t empty_slots;
sem_t full_slots;
pthread_mutex_t mutex_lock;
bool simulation_running = true;

// Thread Handles
pthread_t prod1, prod2, cons;
int id1 = 1, id2 = 2;

// --- UI HELPER FUNCTIONS ---
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void printHeader() {
    // Green and White theme for a Matrix/Hacker aesthetic
    cout << "\033[32m\033[1m"; 
    cout << R"(
    ___   _____   _____  ____  __  __ _   _ _        _ _____ ___  ____  
   / _ \ / ____| / ____||_ _||  \/  | | | | |      / \_   _/ _ \|  _ \ 
  | | | |\___ \  | (___   | | | |\/| | | | | |     / _ \ | || | | | |_) |
  | |_| | ___) |  \___ \  | | | |  | | |_| | |___ / ___ \| || |_| |  _ < 
   \___/ |____/  |____/  |___||_|  |_|\___/|_____/_/   \_\_| \___/|_| \_\
  ========================================================================
    DEVELOPER: ARSAL NAVEED  //  ID: F2023266900  //  V: 2.0.4
    ========================================================================
    )" << "\033[0m\n";
}

void display_system_state() {
    cout << "\n\033[33m[ SYSTEM DIAGNOSTICS ]\033[0m" << endl;
    cout << "\033[1m> Buffer Load:\033[0m     " << buffer.size() << " / " << BUFFER_SIZE << endl;
    cout << "\033[1m> Active Queue:\033[0m    " << ready_queue.size() << " nodes" << endl;
    cout << "\033[1m> Halted Queue:\033[0m    " << blocked_queue.size() << " nodes" << endl;
    
    if (!available_resources.empty()) {
        cout << "\033[1m> Resource Array:\033[0m  R1:" << available_resources[0] 
             << " | R2:" << available_resources[1] 
             << " | R3:" << available_resources[2] << endl;
    }
    cout << "\n\033[32mPress [ENTER] to return to terminal...\033[0m";
    cin.ignore();
    cin.get();
}

int main() {
    // 1. Initialization
    srand(time(0));
    initialize_system_resources();
    sem_init(&empty_slots, 0, BUFFER_SIZE);
    sem_init(&full_slots, 0, 0);
    pthread_mutex_init(&mutex_lock, NULL);

    // 2. Start Worker Threads
    pthread_create(&prod1, NULL, producer_thread, &id1);
    pthread_create(&prod2, NULL, producer_thread, &id2);
    pthread_create(&cons, NULL, consumer_thread, NULL);

    // 3. Command Line Interface
    string cmd;
    while (true) {
        clearScreen();
        printHeader();

        cout << "\n  \033[1mMAIN CONSOLE\033[0m" << endl;
        cout << "  ------------" << endl;
        cout << "  (1) VIEW_STATE     - Inspect memory & queues" << endl;
        cout << "  (2) RUN_SCHED      - Execute priority scheduling" << endl;
        cout << "  (3) INJECT_PROC    - Force add manual process" << endl;
        cout << "  (R) REFRESH        - Update console view" << endl;
        cout << "  (X) SHUTDOWN       - Terminate all threads" << endl;
        
        cout << "\n\033[32mroot@arsal-naveed\033[0m:\033[34m/system\033[0m# ";
        cin >> cmd;

        if (cmd == "1") {
            display_system_state();
        }
        else if (cmd == "2") {
            cout << "\033[33m[!] Initializing scheduler sequence...\033[0m" << endl;
            pthread_mutex_lock(&mutex_lock);
            run_scheduler();
            pthread_mutex_unlock(&mutex_lock);
            this_thread::sleep_for(chrono::milliseconds(1500));
        }
        else if (cmd == "3") {
            Process p;
            p.id = 777; p.burst_time = 4; p.priority = 2;
            p.remaining_time = 4;
            p.arrival_time = time(NULL);
            for(int i=0; i<3; i++) { p.max_need[i]=1; p.need[i]=1; p.allocated[i]=0; }
            
            pthread_mutex_lock(&mutex_lock);
            buffer.push(p); 
            pthread_mutex_unlock(&mutex_lock);
            sem_post(&full_slots);
            cout << "\033[32m[SUCCESS] Manual Process P777 added.\033[0m" << endl;
            this_thread::sleep_for(chrono::milliseconds(1000));
        }
        else if (cmd == "x" || cmd == "X") {
            simulation_running = false;
            sem_post(&empty_slots); sem_post(&full_slots);
            pthread_cancel(prod1); pthread_cancel(prod2); pthread_cancel(cons);
            sem_destroy(&empty_slots);
            sem_destroy(&full_slots);
            pthread_mutex_destroy(&mutex_lock);
            cout << "\033[31m[HALT] All systems terminated.\033[0m" << endl;
            return 0;
        }
    }
    return 0;
}
