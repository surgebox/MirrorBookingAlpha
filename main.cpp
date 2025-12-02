#include <iostream>
#include <string>
#include <functional>
#include <vector>
#include  <fstream>
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
    display, //display all appointments
    help //show help information
};

cmdType getCommandCode(const std::string& input) { // Map string commands to enum values for easier switch-case handling (as strings can't be used in switch)
    if (input == "exit") return cmdType::exit;
    else if (input == "add") return cmdType::add; 
    else if (input == "del") return cmdType::del; 
    else if (input == "reschedule") return cmdType::reschedule;
    else if (input == "display") return cmdType::display;
    else if (input == "help") return cmdType::help;
    else throw std::invalid_argument("Unknown command");
}

// display help info and how to use commands
void displayHelp() {
    std::cout << "\n=====  MirrorBooking Commands  =====\n" << std::endl;
    
    std::cout << "add <name> <time> <service> [date]" << std::endl;
    std::cout << " Add a new appointment" << std::endl;
    std::cout << " time: specific time (e.g., 10am, 2:30pm) or 'next' for next available slot" << std::endl;
    std::cout << " service: 'hair' (30min), 'beard' (15min), 'full' (45min), or custom minutes assigned via number" << std::endl;
    std::cout << " date: optional, defaults to today (format: YYYY-MM-DD)" << std::endl;
    std::cout << " Examples:" << std::endl;
    std::cout << "   add Henry 10am hair (add appointment with customer 'Henry', 30 min hair appoint, today)" << std::endl;
    std::cout << "   add John next beard (add appointment with customer 'John', 15 min beard appoint, next available slot today)" << std::endl;
    std::cout << "   add Jane 2pm full 2025-12-15 (add appointment with customer 'Jane', 45 min full service, on 2025-12-15)" << std::endl;
    std::cout << std::endl;
    
    std::cout << "del <name> <time>" << std::endl;
    std::cout << " Delete an appointment - NOTE: both <name> and <time> are required, use display to find exact time and name" << std::endl;
    std::cout << " Example: del Henry 10am (delete appointment for Henry at 10am today)" << std::endl;
    std::cout << std::endl;
    
    std::cout << "reschedule <name> <oldTime> <newTime> [newDate]" << std::endl;
    std::cout << " Reschedule an existing appointment" << std::endl;
    std::cout << " newTime: specific time or 'next' for next available slot" << std::endl;
    std::cout << " Examples:" << std::endl;
    std::cout << "   reschedule Henry 10am 2pm (reschedule appointment for Henry from 10am to 2pm today)" << std::endl;
    std::cout << "   reschedule John 10am next (reschedule appointment for John from 10am to next available slot today)" << std::endl;
    std::cout << "   reschedule Jane 2pm 3pm 2025-12-15 (reschedule appointment for Jane from 2pm to 3pm on 2025-12-15) " << std::endl;
    std::cout << std::endl;
    
    std::cout << "display [view]" << std::endl;
    std::cout << " Display schedule in different views" << std::endl;
    std::cout << " view options:" << std::endl;
    std::cout << "   daily (default) - Show today's detailed schedule" << std::endl;
    std::cout << "   weekly - Show current week overview" << std::endl;
    std::cout << "   weekly next - Show next week" << std::endl;
    std::cout << "   weekly prev - Show previous week" << std::endl;
    std::cout << "   YYYY-MM-DD - Show specific date" << std::endl;
    std::cout << " Examples:" << std::endl;
    std::cout << "   display (display today's detailed schedule)" << std::endl;
    std::cout << "   display weekly (display current week's overview)" << std::endl;
    std::cout << "   display 2025-12-15 (display schedule for specific date)" << std::endl;
    std::cout << std::endl;
    
    std::cout << "help" << std::endl;
    std::cout << "  Show this help message" << std::endl;
    std::cout << std::endl;
    
    std::cout << "exit" << std::endl;
    std::cout << "  Save and exit the program" << std::endl;
    std::cout << std::endl;
}

// Get current date as YYYY-MM-DD format
std::string getCurrentDate() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm* now_tm = std::localtime(&now_c );
    
    std::ostringstream oss;
    oss << std::put_time(now_tm, "%Y-%m-%d");
    return oss.str();
}

// Converts date from YYYY-MM-DD to MM-DD-YY format (personal preference)
std::string formatDateDisplay(const std::string& date) {
    std::tm tm = {};
    std::istringstream ss(date);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << (tm.tm_mon + 1) << "-"
        << std::setw(2) << tm.tm_mday << "-"
        << std::setw(2) << (tm.tm_year % 100);
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

// Add days to a date
std::string addDaysToDate(const std::string& date, int days) {
    std::tm tm = {};
    std::istringstream ss(date);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    
    std::time_t time = std::mktime(&tm); 
    time += days * 24 * 60 * 60;
    std::tm* result_tm = std::localtime(&time);
    
    std::ostringstream oss;
    oss << std::put_time(result_tm, "%Y-%m-%d");
    return oss.str();
}

// Get day of week name
std::string getDayOfWeek(const std::string& date) {
    std::tm tm = {};
    std::istringstream ss(date);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    std::mktime(&tm); //'normalize' tm struct
    
    const char* days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    return days[tm.tm_wday];
}

// Get start of current week (Monday)
std::string getWeekStart() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm* now_tm = std::localtime(&now_c);
    
    int daysToMonday = (now_tm->tm_wday == 0) ? 6 : now_tm->tm_wday - 1;
    std::time_t monday = now_c - (daysToMonday * 24 * 60 * 60);
    std::tm* monday_tm = std::localtime(&monday);
    
    std::ostringstream oss;
    oss << std::put_time(monday_tm, "%Y-%m-%d");
    return oss.str();
}// Allows strings like "hair"/"beard"/"full"/"both"  to be converted to duration in minutes
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

// Convert time string (e.g., "10am" or "10:30") to minutes since midnight, used for calculations
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
    
    int aStart = timeToMinutes(a.time); // convert time to minutes since midnight
    int aEnd = aStart + a.duration; // calculate end time
    int bStart = timeToMinutes(b.time); // convert time to minutes since midnight
    int bEnd = bStart + b.duration; // calculate end time
    
    return (aStart < bEnd && aEnd > bStart); // checks for overlap
}

// Get current time in minutes since midnight
int getCurrentTimeInMinutes() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm* now_tm = std::localtime(&now_c);
    return now_tm->tm_hour * 60 + now_tm->tm_min;
}

// Display daily schedule
void displayDailySchedule(const std::vector<Appointment>& appointments, const std::string& date) {
    int businessStart = 10 * 60; // 10am
    int businessEnd = 18 * 60;   // 6pm
    int interval = 15;           // 15-minute intervals
    
    std::cout << "\n======= Schedule for today: "  << "(" << getDayOfWeek(date) << ") " << formatDateDisplay(date) << " =======\n" << std::endl;
    
    // get appointments for this date, sorted by time
    std::vector<Appointment> dayAppts;
    for (const auto& apt : appointments) {
        if (apt.date == date) {
            dayAppts.push_back(apt);
        }
    }
    
    // sort by chronological order
    std::sort(dayAppts.begin(), dayAppts.end(), [](const Appointment& a, const Appointment& b) {
        return timeToMinutes(a.time) < timeToMinutes(b.time);
    });
    
    // Determine the actual display range (include after-hours appointments)
    int displayStart = businessStart;
    int displayEnd = businessEnd;
    
    for (const auto& apt : dayAppts) {
        int aptStart = timeToMinutes(apt.time);
        int aptEnd = aptStart + apt.duration;
        if (aptStart < displayStart) displayStart = aptStart;
        if (aptEnd > displayEnd) displayEnd = aptEnd;
    }
    
    // Round to nearest interval
    displayStart = (displayStart / interval) * interval;
    displayEnd = ((displayEnd + interval - 1) / interval) * interval;
    
    // Display appointments and availability blocks
    int currentTime = displayStart;
    
    for (size_t i = 0; i < dayAppts.size(); ++i) {
        const auto& apt = dayAppts[i];
        int aptStart = timeToMinutes(apt.time);
        int aptEnd = aptStart + apt.duration;
        
        // If there's a gap before this appointment, show availability block
        if (currentTime < aptStart) {
            std::string startStr = minutesToTime(currentTime);
            std::string endStr = minutesToTime(aptStart);
            std::string rangeStr = startStr + "-" + endStr;
            std::cout << std::setw(11) << std::left << rangeStr << " | [available]" << std::endl;
        }
        
        // Display the appointment
        std::string timeStr = minutesToTime(aptStart);
        std::cout << std::setw(11) << std::left << timeStr << " | ";
        
        bool isAfterHours = (aptStart < businessStart || aptStart >= businessEnd);
        if (isAfterHours) {
            std::cout << "[OUTSIDE-HOURS] " << apt.name << " - " << apt.service << " (" << apt.duration << " min)";
        } else {
            std::cout << "[BOOKED] " << apt.name << " - " << apt.service << " (" << apt.duration << " min)";
        }
        std::cout << std::endl;
        
        currentTime = aptEnd; // Move to end of this appointment
    }
    
    // If there's time remaining after the last appointment
    if (currentTime < displayEnd) {
        std::string startStr = minutesToTime(currentTime);
        std::string endStr = minutesToTime(displayEnd);
        std::string rangeStr = startStr + "-" + endStr;
        std::cout << std::setw(11) << std::left << rangeStr << " | [available]" << std::endl;
    }
    
    std::cout << "\n" << dayAppts.size() << " appointment(s) scheduled." << std::endl;
}

// Display weekly schedule
void displayWeeklySchedule(const std::vector<Appointment>& appointments, const std::string& startDate) {
    std::cout << "\n===== Weekly Schedule (" << startDate << " to " 
              << addDaysToDate(startDate, 6) << ") =====\n" << std::endl;
    
    for (int day = 0; day < 7; ++day) {
        std::string currentDate = addDaysToDate(startDate, day);
        std::string dayName = getDayOfWeek(currentDate);
        
        // Get appointments for this day
        std::vector<Appointment> dayAppts;
        for (const auto& apt : appointments) {
            if (apt.date == currentDate) {
                dayAppts.push_back(apt);
            }
        }
        
        // Sort by time
        std::sort(dayAppts.begin(), dayAppts.end(), [](const Appointment& a, const Appointment& b) {
            return timeToMinutes(a.time) < timeToMinutes(b.time);
        });
        
        std::cout << std::left << std::setw(12) << dayName << " (" << currentDate << "):  ";
        
        if (dayAppts.empty()) {
            std::cout << "[No appointments]";
        } else {
            // Show appointments in compact format
            for (size_t i = 0; i < dayAppts.size(); ++i) {
                if (i > 0) std::cout << " |";
                std::cout << dayAppts[i].time << "-" << dayAppts[i].name;
            }
            std::cout << " (" << dayAppts.size() << " total)";
        }
        
        std::cout << std::endl;
    }
    
    std::cout << "\nNavigation: 'display weekly next' or 'display weekly prev'" << std::endl;
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
    while (std::getline(file, line)) { // each line represents an appointment, using '|' as delimiter to separate fields
        std::istringstream iss(line); 
        Appointment apt;
        std::string durationStr;
        if (std::getline(iss, apt.name, '|') &&  
            std::getline(iss, apt.time, '|') &&  
            std::getline(iss, apt.date, '|') &&
            std::getline(iss, apt.service, '|') &&
            std::getline(iss, durationStr)) { 
            apt.duration = std::stoi(durationStr);
            appointments.push_back(apt); // add to appointments list
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
        std::getline(std::cin, input); // get full line input

        //holds parsed command and arguments
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
                        std::cerr << "  time: time (ex: 10am) or 'next' for next available" << std::endl;
                        std::cerr << "  service: hair/beard/full or minutes (ex: 30)" << std::endl;
                        std::cerr << "  date: optional (YYYY-MM-DD), defaults to today" << std::endl;
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
                    
                    // handle 'next' time slot for quick booking of soonest available
                    if (timeInput == "next") {
                        apt.time = findNextAvailableTime(appointments, apt.date, apt.duration);
                        
                        // if no slot available for today, offer next day or admin override
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
                    
                    // check for overlaps, return error if true
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

                case cmdType::del: {
                    // takes: "name time" (e.g., "Henry 10am")
                    std::istringstream iss(args);
                    std::string name, time;
                    
                    if (!(iss >> name >> time)) {
                        std::cerr << "Error: Invalid format. Use: del <name> <time> (Use display command to find your appointment details)" << std::endl;
                        break;
                    }
                    
                    // find and delete the appointment
                    bool found = false;
                    for (auto it = appointments.begin(); it != appointments.end(); ++it) {
                        if (it->name == name && it->time == time) {
                            std::cout << "Deleted appointment: " << it->name << " at " << it->time 
                                      << " on " <<  it->date << " (" << it->service << ", " 
                                      << it->duration << " min)" << std::endl;
                            appointments.erase(it); // remove from list
                            saveAppointments(appointments, filename); // save changes
                            found = true;
                            break;
                        }
                    }
                    
                    if (!found) {
                        std::cerr << "Error: No appointment found for " << name << " at " << time << std::endl;
                    }
                    break;
                }

                case cmdType::reschedule: {
                    // uses: "name oldTime newTime [newDate]"
                    // examples: "Henry 10am 2pm", "John 10am next", "Jane 2pm 3pm 2025-12-05"
                    std::istringstream iss(args);
                    std::string name, oldTime, newTimeInput, newDateInput;
                    
                    if (!(iss >> name >> oldTime >> newTimeInput)) {
                        std::cerr << "Error: Invalid format. Use: reschedule <name> <oldTime> <newTime> [newDate]" << std::endl;
                        std::cerr << "  Examples: reschedule Henry 10am 2pm, (where 10am appointment is rescheduled to 2pm)" << std::endl;
                        std::cerr << "            reschedule John 10am next (where 10am is rescheduled to the next available slot)" << std::endl;
                        break;
                    }
                    
                    // find the existing appointment
                    auto it = std::find_if(appointments.begin(), appointments.end(),
                        [&name, &oldTime](const Appointment& apt) {
                            return apt.name == name && apt.time == oldTime;
                        });
                    
                    if (it == appointments.end()) {
                        std::cerr << "Error: No appointment found for " << name << " at " << oldTime << std::endl;
                        break;
                    }
                    
                    // store original appointment details in temp variables
                    Appointment original = *it;
                    Appointment rescheduled = *it;
                    
                    if (iss >> newDateInput) {
                        rescheduled.date = newDateInput;
                    }
                    
                    // if 'next' is specified, find next available slot
                    if (newTimeInput == "next") {
                        appointments.erase(it);
                        
                        rescheduled.time = findNextAvailableTime(appointments, rescheduled.date, rescheduled.duration);
                        
                        if (rescheduled.time.empty()) { // no slots available, offer options
                            std::string nextDay = getNextDate(rescheduled.date);
                            std::cout << "No available slots for " << rescheduled.date << ". Options:" << std::endl;
                            std::cout << "  1. Book for next day ( " << nextDay << ")" << std::endl;
                            std::cout << "  2. Admin override (book after hours)" << std::endl;
                            std::cout << "  3. Cancel reschedule"  << std::endl;
                            std::cout << "Choose  (1/2/3):";
                            
                            std::string choice;
                            std::getline(std::cin, choice);
                            
                            if (choice == "1") {
                                rescheduled.date = nextDay;
                                rescheduled.time = findNextAvailableTime(appointments, rescheduled.date, rescheduled.duration);
                                if (rescheduled.time.empty()) {
                                    std::cerr << "Error: No available time slots for " << rescheduled.date << std::endl;
                                    appointments.push_back(original); // Restore original
                                    break;
                                }
                            } else if (choice == "2") {
                                rescheduled.time = findNextAvailableTime(appointments, rescheduled.date, rescheduled.duration, true);
                                if (rescheduled.time.empty()) {
                                    std::cerr << "Error: No available time slots even with override." << std::endl;
                                    appointments.push_back(original); // Restore original
                                    break;
                                }
                                std::cout << "[Admin Override] Booking after hours." << std::endl;
                            } else {
                                std::cout << "Reschedule cancelled." << std::endl;
                                appointments.push_back(original); // Restore original
                                break;
                            }
                        }
                        
                        // add back the rescheduled appointment
                        appointments.push_back(rescheduled);
                    } else {
                        rescheduled.time = newTimeInput; //new specific time
                        appointments.erase(it); // remove original to check for overlaps
                        
                        // check for overlaps with new time
                        bool hasOverlap = false;
                        for (const auto& existing : appointments) {
                            if (appointmentsOverlap(rescheduled, existing)) {
                                std::cerr << "Error: New time overlaps with existing appointment for " 
                                          << existing.name << " at " << existing.time << std::endl;
                                hasOverlap = true;
                                break;
                            }
                        }
                        
                        if (hasOverlap) {
                            appointments.push_back(original); // restore original appointment
                            break;
                        }
                        
                        
                        appointments.push_back(rescheduled);// add rescheduled appointment
                    }
                    
                    saveAppointments(appointments, filename);
                    std::cout << "Rescheduled appointment: " << original.name << " from " 
                              << original.time << " (" << original.date << ") to " 
                              << rescheduled.time << " (" << rescheduled.date << ")" << std::endl;
                    break;
                }

                case cmdType::help:
                    displayHelp();
                    break;

                case cmdType::display: {
                    // parse args: optional "daily", "weekly", "weekly next", "weekly prev", or date
                    std::istringstream iss(args);
                    std::string viewType, navigation;
                    iss >> viewType;
                    iss >> navigation; // Optional second argument
                    if (viewType.empty() || viewType == "daily") {
                        // display today's schedule by default
                        displayDailySchedule(appointments, getCurrentDate());
                    } else if (viewType == "weekly" || viewType == "week") {
                        // Handle weekly navigation
                        static std::string currentWeekStart = getWeekStart();
                        
                        if (navigation == "next") {
                            currentWeekStart = addDaysToDate(currentWeekStart, 7);
                        } else if (navigation == "prev" || navigation == "previous") {
                            currentWeekStart = addDaysToDate(currentWeekStart, -7);
                        } else if (navigation.empty()) {
                            // Reset to current week if no navigation specified
                            currentWeekStart = getWeekStart();
                        }
                        
                        displayWeeklySchedule(appointments, currentWeekStart);
                    } else if (viewType.find("-") != std::string::npos) {
                        // specific date in YYYY-MM-DD format
                        displayDailySchedule(appointments, viewType);
                    } else {
                        std::cerr << "Error: Invalid display option. Use 'daily', 'weekly [next|prev]', or a date (YYYY-MM-DD)" << std::endl;
                    }
                    break;
                }
            }

        } catch (const std::invalid_argument& e) {
            std::cerr << "Error: " << e.what() << std::endl; // handle unknown command
        }
    }
}