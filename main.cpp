#include <iostream>
#include <string>
#include <functional>
#include <vector>
#include <fstream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <iomanip>

 struct Appointment { // Structure to hold appointment details
     std::string name;     //client name
     std::string time;     //appointment time (e.g., "10:00")
     std::string date;     //appointment date (YYYY-MM-DD)
     std::string service;  //service type (hair, beard, full)
     int duration;         //duration in minutes
 };

enum class cmdType {
    exit, //exit the program
    add,    //add a new appointment
    del,    //delete an appointment
    reschedule, //reschedule existing appointment
    display //display all appointments
};

cmdType getCommandCode(const std::string& input) { // Map string commands to enum values for easier switch-case handling (as strings can't be used in switch)
    if (input == "exit") return cmdType::exit;
    else if (input == "add") return cmdType::add; 
    else if (input == "del") return cmdType::del; 
    else if (input == "reschedule") return cmdType::reschedule;
    else if (input == "display") return cmdType::display;
    else throw std::invalid_argument("Unknown command");
}

// Get current date as YYYY-MM-DD string
std::string getCurrentDate() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm* now_tm = std::localtime(&now_c);
    
    std::ostringstream oss;
    oss << std::put_time(now_tm, "%Y-%m-%d");
    return oss.str();
}

// Get next day's date as YYYY-MM-DD string
std::string getNextDate(const std::string& date) {
    // Simple implementation: parse YYYY-MM-DD and add one day 
    std::tm tm = {};
    std::istringstream ss(date);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    
    std::time_t time = std::mktime(&tm);
    time += 24 * 60 * 60; // Add one day
    std::tm* next_tm = std::localtime(&time);
    
    std::ostringstream oss;
    oss << std::put_time(next_tm, "%Y-%m-%d");
    return oss.str();
}

// Allows strings like "hair"/"beard"/"full"/"both"  to be converted to duration in minutes
int parseServiceDuration(const std::string& service) {
    if (service == "hair" || service == "haircut") return 30;
    if (service == "beard") return 15;
    if (service == "full" || service == "both") return 45;
    
    // Try to parse as number
     try {
        return std::stoi(service);
    } catch (...) {
        return -1; // Invalid
    }
}

// Convert time string (e.g., "10am" or "10:30") to minutes since midnight
int timeToMinutes(const std::string& timeStr) {
    std::string time = timeStr;
    bool isPM = false;
    
    // Check for am/pm and cleans the string
    if (time.find("pm") != std::string::npos || time.find("PM") != std::string::npos) {
        isPM = true;
        time.erase(std::remove_if(time.begin(), time.end(), 
            [](char c) { return !std::isdigit(c) && c != ':'; }), time.end());
    } else if (time.find("am") != std::string::npos || time.find("AM") != std::string::npos) {
        time.erase(std::remove_if(time.begin(), time.end(), 
            [](char c) { return !std::isdigit(c) && c != ':'; }), time.end());
    }
    
    int hours = 0, minutes = 0;
    size_t colonPos = time.find(':');
    if (colonPos != std::string::npos) {
        hours = std::stoi(time.substr(0, colonPos));
        minutes = std::stoi(time.substr(colonPos + 1));
    } else {
        hours = std::stoi(time);
    }
    
    if (isPM && hours != 12) hours += 12;
    if (!isPM && hours == 12) hours = 0;
    
    return hours * 60 + minutes;
}

// Convert minutes since midnight to time string (e.g. "10am" or "10:30" 
std::string minutesToTime(int minutes) {
    int hours = minutes / 60;
    int mins = minutes % 60;
    std::string period = (hours >= 12) ? "pm" : "am"; // Determine am/pm based on hours
    if (hours > 12) hours -= 12;
    if (hours == 0) hours = 12;
    
    std::ostringstream oss;
    oss << hours;  //
    if (mins > 0) oss << ":" << std::setfill('0') << std::setw(2) << mins; // appends minutes with leading zero if needed
    oss << period; // append am/pm
    return oss.str();
}

// check if two appointments overlap with eachother to prevent double booking
bool appointmentsOverlap(const Appointment& a, const Appointment& b) {
    if (a.date != b.date) return false;
    
    int aStart = timeToMinutes(a.time);
    int aEnd = aStart + a.duration;
    int bStart = timeToMinutes(b.time);
    int bEnd = bStart + b.duration;
    
    return (aStart < bEnd && aEnd > bStart);
}

// Get current time in minutes since midnight
int getCurrentTimeInMinutes() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm* now_tm = std::localtime(&now_c);
    return now_tm->tm_hour * 60 + now_tm->tm_min;
}

// Find next available time slot (with optional admin override)
std::string findNextAvailableTime(const std::vector<Appointment>& appointments, 
                                  const std::string& date, int duration, bool adminOverride = false) {
    int businessStart = 10 * 60; // 10am
    int businessEnd = adminOverride ? 22 * 60 : 18 * 60;  // 10pm with override, 6pm normally
    int interval = 15;      // check every 15 minutes
    
    // get current time if booking for today
    std::string today = getCurrentDate();
    int startTime = businessStart;
    
    if (date == today) {
        int currentTime = getCurrentTimeInMinutes();
        // round up to next 15-minute interval
        currentTime = ((currentTime + interval - 1) / interval) * interval;
        // use the later of current time or business start
        startTime = (currentTime > businessStart) ? currentTime : businessStart;
    }
    
    for (int currentTime = startTime; currentTime + duration <= businessEnd; currentTime += interval) {
        bool available = true;
        
        for (const auto& apt : appointments) {
            if (apt.date == date) {
                int aptStart = timeToMinutes(apt.time);
                int aptEnd = aptStart + apt.duration;
                int slotEnd = currentTime + duration;
                
                if (currentTime < aptEnd && slotEnd > aptStart) {
                    available = false;
                    break;
                }
            }
        }
        
        if (available) {
            return minutesToTime(currentTime);
        }
    }
    
    return ""; // no available slot available   
}

// Load appointments from file
void loadAppointments(std::vector<Appointment>& appointments, const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return; // file doesn't exist yet
    }
    
    std::string line;
    while (std::getline(file, line)) { // Each line represents an appointment, using '|' as delimiter
        std::istringstream iss(line); 
        Appointment apt;
        std::string durationStr;
        if (std::getline(iss, apt.name, '|') &&  
            std::getline(iss, apt.time, '|') &&  
            std::getline(iss, apt.date, '|') &&
            std::getline(iss, apt.service, '|') &&
            std::getline(iss, durationStr)) { 
            apt.duration = std::stoi(durationStr);
            appointments.push_back(apt); // Add to appointments list
        }
    }
    file.close(); // close the file after reading to avoid corruption
}

// Save appointments to file "appointments.txt"
void saveAppointments(const std::vector<Appointment>& appointments, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file for writing." << std::endl;
        return;
    }
    
    for (const auto& apt : appointments) {
        file << apt.name << "|" << apt.time << "|" << apt.date << "|"
             << apt.service << "|" << apt.duration << std::endl;
    }
    file.close();
}

int main(){
    std::cout << std::unitbuf; // make sure output is displayed immediately
    std::cerr << std::unitbuf; // make sure error messages are displayed immediately

    std::vector<Appointment> appointments; // store appointments
    const std::string filename = "appointments.txt";
    
    // Load existing appointments from file
    loadAppointments(appointments, filename);

    while(true){
        std::cout << "$";
        std::string input;
        std::getline(std::cin, input);

        // parse command and arguments
        std::string command;
        std::string args;

        size_t spacePos = input.find(' '); // find first space to separate command and follow up arguments
        if (spacePos != std::string::npos) {
            command = input.substr(0, spacePos);
            args = input.substr(spacePos + 1);
        } else {
            command = input;
            args = "";
        }



        try { 
            switch(getCommandCode(command)){
                case cmdType::exit:
                    saveAppointments(appointments, filename);
                    std::cout << "Exiting program." << std::endl;
                    return 0;
                case cmdType::add: {
                    //  args order: "name time service [date]"
                    // Examples: "Henry 10am hair", "John next beard", "Jane 2pm full 2025-12-01"
                    std::istringstream iss(args);
                    Appointment apt;
                    std::string timeInput, serviceInput, dateInput;
                    
                    if (!(iss >> apt.name >> timeInput >> serviceInput)) { // Mandatory fields
                        std::cerr << "Error: Invalid format. Use: add <name> <time> <service> <date>" << std::endl;
                        std::cerr << "  time: time (e.g., 10am) or 'next' for next available" << std::endl;
                        std::cerr << "  service: hair/beard/full or minutes (e.g., 30)" << std::endl;
                        std::cerr << "  date: optional, defaults to today (YYYY-MM-DD)" << std::endl;
                        break;
                    }
                    
                    // parse service and duration
                    apt.duration = parseServiceDuration(serviceInput);
                    if (apt.duration <= 0) {
                        std::cerr << "Error: Invalid service. Use 'hair', 'beard', 'full', or a number of minutes." << std::endl;
                        break;
                    }
                    apt.service = serviceInput;
                    
                    // parse date (optional, default:today)
                    if (iss >> dateInput) {
                        apt.date = dateInput;
                    } else {
                        apt.date = getCurrentDate();
                    }
                    
                    // Handle 'next' time slot
                    if (timeInput == "next") {
                        apt.time = findNextAvailableTime(appointments, apt.date, apt.duration);
                        
                        // If no slot available for today, offer next day or admin override
                        if (apt.time.empty() && apt.date == getCurrentDate()) {
                            std::string nextDay = getNextDate(apt.date);
                            std::cout << "No available slots for today. Options:" << std::endl;
                            std::cout << "  1. Book for next day (" << nextDay << ")" << std::endl;
                            std::cout << "  2. Admin override (book after hours)" << std::endl;
                            std::cout << "  3. Cancel" << std::endl;
                            std::cout << "Choose (1/2/3): ";
                            
                            std::string choice;
                            std::getline(std::cin, choice);
                            
                            if (choice == "1") { // book for next day
                                apt.date = nextDay;
                                apt.time = findNextAvailableTime(appointments, apt.date, apt.duration);
                                if (apt.time.empty()) {
                                    std::cerr << "Error: No available time slots for " << apt.date << std::endl;
                                    break;
                                }
                            } else if (choice == "2") { // admin override
                                apt.time = findNextAvailableTime(appointments, apt.date, apt.duration, true);
                                if (apt.time.empty()) {
                                    std::cerr << "Error: No available time slots even with override." << std::endl;
                                    break;
                                }
                                std::cout << "[Admin Override] Booking after hours." << std::endl;
                            } else { // cancel
                                std::cout << "Booking cancelled." << std::endl;
                                break;
                            }
                        } else if (apt.time.empty()) { // no slots available at all
                            std::cerr << "Error: No available time slots for " << apt.date << std::endl;
                            break;
                        }
                    } else { // specific time provided
                        apt.time = timeInput;
                    }
                    
                    // Check for overlaps
                    bool hasOverlap = false;
                    for (const auto& existing : appointments) {
                        if (appointmentsOverlap(apt, existing)) {
                            std::cerr << "Error: Appointment overlaps with existing appointment for " 
                                      << existing.name << " at " << existing.time << std::endl;
                            hasOverlap = true;
                            break;
                        }
                    }
                    
                    if (!hasOverlap) { // if no overlaps, add appointment
                        appointments.push_back(apt);
                        saveAppointments(appointments, filename);
                        std::cout << "Added appointment: " << apt.name << " at " << apt.time 
                                  << " on " << apt.date << " (" << apt.service << ", " 
                                  << apt.duration << " min)" << std::endl;
                    }
                    break;
                }
                case cmdType::del:
                    std::cout << "Deleting an appointment with args: '" << args << "'..." << std::endl;
                    // Delete appointment logic here
                    break;
                case cmdType::reschedule:
                    std::cout << "Rescheduling an appointment with args: '" << args << "'..." << std::endl;
                    // Reschedule appointment logic here
                    break;
                case cmdType::display:
                    if (appointments.empty()) {
                        std::cout << "No appointments scheduled." << std::endl;
                    } else {
                        std::cout << "\nAppointments:" << std::endl;
                        for (size_t i = 0; i < appointments.size(); ++i) {
                            std::cout << i + 1 << ". " << appointments[i].name 
                                      << " - " << appointments[i].time 
                                      << " on " << appointments[i].date
                                      << " (" << appointments[i].service << ", "
                                      << appointments[i].duration << " min)" << std::endl;
                        }
                    }
                    break;
            }
        } catch (const std::invalid_argument& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
}