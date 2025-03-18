#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "cJSON.h"
#include <locale.h>
#include <time.h>
#include <string.h>
#include <math.h>

// Linking the Winsock library
#pragma comment(lib, "Ws2_32.lib")

// Constants
#define API_KEY "a616f317d8bb992d2658893c28603ab2"
#define HISTORY_FILE "history.txt"
#define WEATHER_CHANGES_FILE "weather_table.txt"
#define BUFFER_SIZE 8192

// Function prototypes
void write_to_file(const char* filename, const char* data);
void write_weather_table(const char* filename, const char* label, float temp, float feels_like, int humidity, int pressure, float wind_speed, int wind_deg, const char* wind_direction);
void get_current_time(char* buffer, int size);
void get_weather_data(float latitude, float longitude, const char* api_key);
void get_weather_data_by_city(const char* city_name, const char* api_key);
void display_history();
void display_weather_changes();
const char* get_wind_direction(int degrees);

int main() {
    setlocale(LC_ALL, "us_US");
    system("chcp 65001 > nul");

    while (1) {
        int choice;

        printf("\n=== Weather Application ===\n");
        printf("Choose an option:\n");
        printf("1. Get weather data by coordinates\n");
        printf("2. Get weather data by city name\n");
        printf("3. View query history\n");
        printf("4. View weather changes table\n");
        printf("5. Exit\n");
        printf("Your choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Input error! Please enter a valid number.\n");
            while (getchar() != '\n');
            continue;
        }

        if (choice == 1) {
            float latitude, longitude;

            printf("Enter latitude (example: 50.6721): ");
            if (scanf("%f", &latitude) != 1) {
                printf("Error entering latitude!\n");
                while (getchar() != '\n');
                continue;
            }

            printf("Enter longitude (example: 17.9253): ");
            if (scanf("%f", &longitude) != 1) {
                printf("Error entering longitude!\n");
                while (getchar() != '\n');
                continue;
            }

            get_weather_data(latitude, longitude, API_KEY);
        }
        else if (choice == 2) {
            char city_name[100];

            printf("Enter city name (example: London): ");
            scanf("%99s", city_name);

            get_weather_data_by_city(city_name, API_KEY);
        }
        else if (choice == 3) {
            display_history();
        }
        else if (choice == 4) {
            display_weather_changes();
        }
        else if (choice == 5) {
            printf("Exiting the program... Goodbye!\n");
            break;
        }
        else {
            printf("Invalid choice! Please select a valid option.\n");
        }

        printf("\nPress Enter to return to the menu...\n");
        while (getchar() != '\n');
        getchar();
        system("cls");
    }

    return 0;
}

// Function to write a string to a file
void write_to_file(const char* filename, const char* data) {
    FILE* file = fopen(filename, "a");
    if (file != NULL) {
        fprintf(file, "%s\n", data);
        fclose(file);
    }
    else {
        printf("Error opening file %s\n", filename);
    }
}

// Function to write a weather table entry to a file
void write_weather_table(const char* filename, const char* label, float temp, float feels_like, int humidity, int pressure, float wind_speed, int wind_deg, const char* wind_direction) {
    FILE* file = fopen(filename, "a");
    if (file != NULL) {
        fprintf(file, "%-20s | %-10.1f  | %-10.1f | %-10d | %-10d | %-10.1f | %-10d | %-10s\n",
            label, temp, feels_like, humidity, pressure, wind_speed, wind_deg, wind_direction);
        fclose(file);
    }
    else {
        printf("Error opening file %s\n", filename);
    }
}

// Function to get the current time as a string
void get_current_time(char* buffer, int size) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

// Function to display query history
void display_history() {
    FILE* file = fopen(HISTORY_FILE, "r");
    if (file == NULL) {
        printf("Query history is empty.\n");
        return;
    }

    char line[512];
    printf("\n=== Query History ===\n");
    while (fgets(line, sizeof(line), file)) {
        printf("%s", line);
    }
    printf("=====================\n\n");

    fclose(file);
}

// Function to display weather changes table
void display_weather_changes() {
    FILE* file = fopen(WEATHER_CHANGES_FILE, "r");
    if (file == NULL) {
        printf("Weather changes table is empty.\n");
        return;
    }

    char line[512];
    printf("\n=== Weather Changes Table ===\n");
    printf("Location             | Temperature | Feels Like | Humidity   | Pressure   | Wind Speed | Wind Deg   | Wind Dir   |\n");
    printf("---------------------------------------------------------------------------------------------------------------\n");
    while (fgets(line, sizeof(line), file)) {
        printf("%s", line);
    }
    printf("---------------------------------------------------------------------------------------------------------------\n\n");

    fclose(file);
}

// Get wind direction as a string based on degrees
const char* get_wind_direction(int degrees) {
    const char* directions[] = {
        "N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE",
        "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"
    };
    int index = (int)round((double)degrees / 22.5) % 16;
    return directions[index];
}

// Function to get weather data by coordinates
void get_weather_data(float latitude, float longitude, const char* api_key) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Error initializing sockets\n");
        return;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        printf("Error creating socket\n");
        WSACleanup();
        return;
    }

    struct addrinfo hints = { 0 }, * result = NULL;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo("api.openweathermap.org", "80", &hints, &result) != 0) {
        printf("Error: Failed to get server IP address\n");
        closesocket(sock);
        WSACleanup();
        return;
    }

    if (connect(sock, result->ai_addr, (int)result->ai_addrlen) < 0) {
        printf("Error connecting to server\n");
        freeaddrinfo(result);
        closesocket(sock);
        WSACleanup();
        return;
    }

    freeaddrinfo(result);

    char request[512];
    sprintf_s(request,
        "GET /data/2.5/weather?lat=%.4f&lon=%.4f&appid=%s&lang=en&units=metric HTTP/1.1\r\n"
        "Host: api.openweathermap.org\r\n"
        "Connection: close\r\n\r\n",
        latitude, longitude, api_key);

    send(sock, request, (int)strlen(request), 0);

    char response[8192];
    int bytes_received = 0;
    int total_received = 0;

    while ((bytes_received = recv(sock, response + total_received, sizeof(response) - 1 - total_received, 0)) > 0) {
        total_received += bytes_received;
        if (total_received >= sizeof(response) - 1) {
            break;
        }
    }

    if (bytes_received < 0) {
        printf("Error receiving data from server\n");
        closesocket(sock);
        WSACleanup();
        return;
    }

    response[total_received] = '\0';

    closesocket(sock);
    WSACleanup();

    char* json_start = strstr(response, "\r\n\r\n");
    if (json_start == NULL) {
        printf("Error: Failed to find JSON in response\n");
        return;
    }

    json_start += 4;
    cJSON* json = cJSON_Parse(json_start);
    if (json == NULL) {
        printf("Error: Failed to parse JSON response\n");
        return;
    }

    cJSON* weather = cJSON_GetObjectItemCaseSensitive(json, "weather");
    cJSON* main = cJSON_GetObjectItemCaseSensitive(json, "main");
    cJSON* wind = cJSON_GetObjectItemCaseSensitive(json, "wind");

    if (weather && cJSON_IsArray(weather) && main && wind) {
        cJSON* weather_item = cJSON_GetArrayItem(weather, 0);
        cJSON* description = cJSON_GetObjectItemCaseSensitive(weather_item, "description");
        cJSON* temp = cJSON_GetObjectItemCaseSensitive(main, "temp");
        cJSON* feels_like = cJSON_GetObjectItemCaseSensitive(main, "feels_like");
        cJSON* humidity = cJSON_GetObjectItemCaseSensitive(main, "humidity");
        cJSON* pressure = cJSON_GetObjectItemCaseSensitive(main, "pressure");
        cJSON* wind_speed = cJSON_GetObjectItemCaseSensitive(wind, "speed");
        cJSON* wind_deg = cJSON_GetObjectItemCaseSensitive(wind, "deg");

        if (description && cJSON_IsString(description) &&
            temp && cJSON_IsNumber(temp) &&
            feels_like && cJSON_IsNumber(feels_like) &&
            humidity && cJSON_IsNumber(humidity) &&
            pressure && cJSON_IsNumber(pressure) &&
            wind_speed && cJSON_IsNumber(wind_speed) &&
            wind_deg && cJSON_IsNumber(wind_deg)) {

            const char* wind_direction = get_wind_direction(wind_deg->valueint);

            printf("\nWeather information for coordinates: %.6f, %.6f\n", latitude, longitude);
            printf("Weather Description: %s\n", description->valuestring);
            printf("Temperature: %.1f°C\n", temp->valuedouble);
            printf("Feels Like: %.1f°C\n", feels_like->valuedouble);
            printf("Humidity: %d%%\n", humidity->valueint);
            printf("Pressure: %d hPa\n", pressure->valueint);
            printf("Wind Speed: %.1f m/s\n", wind_speed->valuedouble);
            printf("Wind Direction: %d° (%s)\n", wind_deg->valueint, wind_direction);

            // Logging the query history
            char time_buffer[20];
            get_current_time(time_buffer, sizeof(time_buffer));
            char log_entry[256];
            sprintf(log_entry, "[%s] Coordinates: %.6f, %.6f, Temperature: %.1f°C, Wind Speed: %.1f m/s, Direction: %s", time_buffer, latitude, longitude, temp->valuedouble, wind_speed->valuedouble, wind_direction);
            write_to_file(HISTORY_FILE, log_entry);

            // Logging weather table
            write_weather_table(WEATHER_CHANGES_FILE, "Coordinates", temp->valuedouble, feels_like->valuedouble, humidity->valueint, pressure->valueint, wind_speed->valuedouble, wind_deg->valueint, wind_direction);

        }
        else {
            printf("Error: Failed to extract necessary data from JSON\n");
        }
    }
    else {
        printf("Error: Failed to retrieve weather data\n");
    }

    cJSON_Delete(json);
}

// Function to get weather data by city name
void get_weather_data_by_city(const char* city_name, const char* api_key) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Error initializing sockets\n");
        return;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        printf("Error creating socket\n");
        WSACleanup();
        return;
    }

    struct addrinfo hints = { 0 }, * result = NULL;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo("api.openweathermap.org", "80", &hints, &result) != 0) {
        printf("Error: Failed to get server IP address\n");
        closesocket(sock);
        WSACleanup();
        return;
    }

    if (connect(sock, result->ai_addr, (int)result->ai_addrlen) < 0) {
        printf("Error connecting to server\n");
        freeaddrinfo(result);
        closesocket(sock);
        WSACleanup();
        return;
    }

    freeaddrinfo(result);

    char request[512];
    sprintf_s(request,
        "GET /data/2.5/weather?q=%s&appid=%s&lang=en&units=metric HTTP/1.1\r\n"
        "Host: api.openweathermap.org\r\n"
        "Connection: close\r\n\r\n",
        city_name, api_key);

    send(sock, request, (int)strlen(request), 0);

    char response[8192]; // Increased buffer size for larger responses
    int bytes_received = 0;
    int total_received = 0;

    // Receiving the full response
    while ((bytes_received = recv(sock, response + total_received, sizeof(response) - 1 - total_received, 0)) > 0) {
        total_received += bytes_received;
        if (total_received >= sizeof(response) - 1) {
            break; // Buffer limit reached
        }
    }

    if (bytes_received < 0) {
        printf("Error receiving data from server\n");
        closesocket(sock);
        WSACleanup();
        return;
    }

    response[total_received] = '\0';

    closesocket(sock);
    WSACleanup();

    char* json_start = strstr(response, "\r\n\r\n");
    if (json_start == NULL) {
        printf("Error: Failed to find JSON in response\n");
        return;
    }

    json_start += 4;
    cJSON* json = cJSON_Parse(json_start);
    if (json == NULL) {
        printf("Error: Failed to parse JSON response\n");
        return;
    }

    cJSON* weather = cJSON_GetObjectItemCaseSensitive(json, "weather");
    cJSON* main = cJSON_GetObjectItemCaseSensitive(json, "main");
    cJSON* wind = cJSON_GetObjectItemCaseSensitive(json, "wind");

    if (weather && cJSON_IsArray(weather) && main && wind) {
        cJSON* weather_item = cJSON_GetArrayItem(weather, 0);
        cJSON* description = cJSON_GetObjectItemCaseSensitive(weather_item, "description");
        cJSON* temp = cJSON_GetObjectItemCaseSensitive(main, "temp");
        cJSON* feels_like = cJSON_GetObjectItemCaseSensitive(main, "feels_like");
        cJSON* humidity = cJSON_GetObjectItemCaseSensitive(main, "humidity");
        cJSON* pressure = cJSON_GetObjectItemCaseSensitive(main, "pressure");
        cJSON* wind_speed = cJSON_GetObjectItemCaseSensitive(wind, "speed");
        cJSON* wind_deg = cJSON_GetObjectItemCaseSensitive(wind, "deg");

        if (description && cJSON_IsString(description) &&
            temp && cJSON_IsNumber(temp) &&
            feels_like && cJSON_IsNumber(feels_like) &&
            humidity && cJSON_IsNumber(humidity) &&
            pressure && cJSON_IsNumber(pressure) &&
            wind_speed && cJSON_IsNumber(wind_speed) &&
            wind_deg && cJSON_IsNumber(wind_deg)) {

            const char* wind_direction = get_wind_direction(wind_deg->valueint);

            printf("\nWeather information for city: %s\n", city_name);
            printf("Weather Description: %s\n", description->valuestring);
            printf("Temperature: %.1f°C\n", temp->valuedouble);
            printf("Feels Like: %.1f°C\n", feels_like->valuedouble);
            printf("Humidity: %d%%\n", humidity->valueint);
            printf("Pressure: %d hPa\n", pressure->valueint);
            printf("Wind Speed: %.1f m/s\n", wind_speed->valuedouble);
            printf("Wind Direction: %d° (%s)\n", wind_deg->valueint, wind_direction);

            // Logging the query history
            char time_buffer[20];
            get_current_time(time_buffer, sizeof(time_buffer));
            char log_entry[256];
            sprintf(log_entry, "[%s] City: %s, Temperature: %.1f°C, Wind Speed: %.1f m/s, Direction: %s", time_buffer, city_name, temp->valuedouble, wind_speed->valuedouble, wind_direction);
            write_to_file(HISTORY_FILE, log_entry);

            // Logging weather table
            write_weather_table(WEATHER_CHANGES_FILE, city_name, temp->valuedouble, feels_like->valuedouble, humidity->valueint, pressure->valueint, wind_speed->valuedouble, wind_deg->valueint, wind_direction);

        }
        else {
            printf("Error: Failed to extract necessary data from JSON\n");
        }
    }
    else {
        printf("Error: Failed to retrieve weather data\n");
    }

    cJSON_Delete(json);
}