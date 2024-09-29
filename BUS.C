#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

// Linked list node for storing tickets
struct TicketNode {
    int pnr;
    int bus_number;
    char journey_date[15];
    char from_station[50];
    char to_station[50];
    int num_passengers;
    struct TicketNode *next; // Linked list pointer
};

// Stack node for undo feature
struct StackNode {
    struct TicketNode *ticket;
    struct StackNode *next;
};

// Queue node for ticket requests
struct QueueNode {
    struct TicketNode *ticket;
    struct QueueNode *next;
};

// Bus structure
struct Bus {
    int bus_number;
    char name[50];
    char source[50];
    char destination[50];
    int distance;
};

// Linked list head for tickets
struct TicketNode *ticket_head = NULL;
struct StackNode *undo_stack = NULL;  // Stack head for undo
struct QueueNode *queue_front = NULL, *queue_rear = NULL;  // Queue front and rear

struct Bus buses[3];

void initializeBuses();
void displayAvailableBuses();
void bookTicket();
void ticketDetails();
void cancelTicket();
void pushUndo(struct TicketNode *ticket);
struct TicketNode* popUndo();
void enqueueTicket(struct TicketNode *ticket);
struct TicketNode* dequeueTicket();
int generatePNR();
int calculateFare(int bus_index, char seat_type, int distance);
bool isQueueEmpty();

int main() {
    int choice;
    initializeBuses();
    printf("Welcome to Bus Booking System\n");

    do {
        printf("\n1. Display Available Buses\n");
        printf("2. Book Ticket\n");
        printf("3. Ticket Details\n");
        printf("4. Cancel Ticket\n");
        printf("5. Undo Cancellation\n");
        printf("6. Process Ticket Queue\n");
        printf("7. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1:
                displayAvailableBuses();
                break;
            case 2:
                bookTicket();
                break;
            case 3:
                ticketDetails();
                break;
            case 4:
                cancelTicket();
                break;
            case 5: {
                struct TicketNode *restored = popUndo();
                if (restored) {
                    printf("Cancellation undone for PNR: %d\n", restored->pnr);
                    restored->next = ticket_head;
                    ticket_head = restored;
                } else {
                    printf("No cancellations to undo.\n");
                }
                break;
            }
            case 6: {
                struct TicketNode *processed_ticket = dequeueTicket();
                if (processed_ticket) {
                    printf("Processed ticket for PNR: %d\n", processed_ticket->pnr);
                    
                    // Add the processed ticket to the linked list (ticket_head)
                    if (ticket_head == NULL) {
                        ticket_head = processed_ticket;
                    } else {
                        struct TicketNode *current = ticket_head;
                        while (current->next != NULL) {
                            current = current->next;
                        }
                        current->next = processed_ticket;
                    }
                } else {
                    printf("No tickets in queue to process.\n");
                }
                break;
            }
            case 7:
                printf("Exiting program.\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 7);

    return 0;
}

// Initialize buses
void initializeBuses() {
    struct Bus bus1 = {1001, "City Express", "City A", "City B", 200};
    struct Bus bus2 = {1002, "Interstate Bus", "City A", "City C", 300};
    struct Bus bus3 = {1003, "Night Rider", "City B", "City D", 400};
    buses[0] = bus1;
    buses[1] = bus2;
    buses[2] = bus3;
}

// Display available buses
void displayAvailableBuses() {
    printf("\nAvailable Buses:\n");
    for (int i = 0; i < 3; i++) {
        printf("Bus Number: %d, Name: %s, From: %s, To: %s, Distance: %d km\n",
               buses[i].bus_number, buses[i].name, buses[i].source,
               buses[i].destination, buses[i].distance);
    }
}

// Book a ticket (linked list + queue for ticket requests)
void bookTicket() {
    int bus_number;
    printf("Enter bus number: ");
    scanf("%d", &bus_number);

    int bus_index = -1;
    for (int i = 0; i < 3; i++) {
        if (buses[i].bus_number == bus_number) {
            bus_index = i;
            break;
        }
    }

    if (bus_index == -1) {
        printf("Invalid bus number.\n");
        return;
    }

    struct TicketNode *new_ticket = (struct TicketNode *)malloc(sizeof(struct TicketNode));
    new_ticket->pnr = generatePNR();
    new_ticket->bus_number = bus_number;

    printf("Enter journey date (DD/MM/YYYY): ");
    scanf("%s", new_ticket->journey_date);

    strcpy(new_ticket->from_station, buses[bus_index].source);
    strcpy(new_ticket->to_station, buses[bus_index].destination);
    new_ticket->next = NULL;

    // Enqueue the ticket request
    enqueueTicket(new_ticket);

    printf("Ticket booked and added to queue. PNR: %d\n", new_ticket->pnr);
}

// Display all booked tickets (linked list)
void ticketDetails() {
    struct TicketNode *current = ticket_head;
    if (current == NULL) {
        printf("No tickets booked yet.\n");
        return;
    }

    printf("\nTicket Details:\n");
    while (current != NULL) {
        printf("PNR: %d, Bus Number: %d, Journey Date: %s, From: %s, To: %s\n",
               current->pnr, current->bus_number, current->journey_date,
               current->from_station, current->to_station);
        current = current->next;
    }
}

// Cancel a ticket (linked list + stack for undo)
void cancelTicket() {
    int pnr;
    printf("Enter PNR to cancel ticket: ");
    scanf("%d", &pnr);

    struct TicketNode *current = ticket_head, *prev = NULL;
    while (current != NULL && current->pnr != pnr) {
        prev = current;
        current = current->next;
    }

    if (current == NULL) {
        printf("Ticket with PNR %d not found.\n", pnr);
        return;
    }

    if (prev == NULL) {
        ticket_head = current->next;
    } else {
        prev->next = current->next;
    }

    pushUndo(current);
    printf("Ticket with PNR %d canceled successfully.\n", pnr);
}

// Generate a random PNR number
int generatePNR() {
    srand(time(NULL));
    return rand() % 100000;
}

// Calculate fare based on seat type
int calculateFare(int bus_index, char seat_type, int distance) {
    switch (seat_type) {
        case 'G': return 5 * distance;  // General
        case 'S': return 10 * distance;  // Sleeper
        case 'A': return 20 * distance;  // AC
        default: return -1;  // Invalid seat type
    }
}

// Stack functions for undo
void pushUndo(struct TicketNode *ticket) {
    struct StackNode *new_node = (struct StackNode *)malloc(sizeof(struct StackNode));
    new_node->ticket = ticket;
    new_node->next = undo_stack;
    undo_stack = new_node;
}

struct TicketNode* popUndo() {
    if (undo_stack == NULL) {
        return NULL;
    }
    struct StackNode *temp = undo_stack;
    struct TicketNode *ticket = temp->ticket;
    undo_stack = undo_stack->next;
    free(temp);
    return ticket;
}

// Queue functions for ticket processing
void enqueueTicket(struct TicketNode *ticket) {
    struct QueueNode *new_node = (struct QueueNode *)malloc(sizeof(struct QueueNode));
    new_node->ticket = ticket;
    new_node->next = NULL;
    if (queue_rear == NULL) {
        queue_front = queue_rear = new_node;
    } else {
        queue_rear->next = new_node;
        queue_rear = new_node;
    }
}

struct TicketNode* dequeueTicket() {
    if (queue_front == NULL) {
        return NULL;
    }
    struct QueueNode *temp = queue_front;
    struct TicketNode *ticket = temp->ticket;
    queue_front = queue_front->next;
    if (queue_front == NULL) {
        queue_rear = NULL;
    }
    free(temp);
    return ticket;
}
