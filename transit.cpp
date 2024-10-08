#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <cmath>
#include <chrono>
#include <functional>
#include <queue>
#include <algorithm>
#include <chrono>


const auto MAX_MINUTES = std::chrono::duration<int64_t, std::ratio<60>>::max();

std::chrono::minutes get_time_point(int hours, int minutes){
    return std::chrono::minutes(hours * 60 + minutes);
}

struct Coordinates {
    double lat;
    double lon;
};

struct Station {
    std::string name;
    Coordinates location;
    bool wheelchair_accessible;
};

struct Connection {
    std::string to;
    double average_speed = 60; //km/h
    std::string mode; // "bus", "train", "walk", etc.
    // std::unordered_map<std::string, std::chrono::minutes> schedule;
    //std::vector<std::chrono::system_clock::time_point> schedule;
    std::vector<std::chrono::minutes> schedule;
};

class Graph {
private:
    std::function<std::pair<std::vector<std::string>, std::chrono::minutes>(const std::string&, const std::string&, const std::chrono::minutes&)> searchAlgo;
    std::unordered_map<std::string, Station> stations;
    std::unordered_map<std::string, std::vector<Connection>> adjacencyList;

    std::pair<std::vector<std::string>, std::chrono::minutes> Dijkstra(const std::string& start, const std::string& end, 
                                      const std::chrono::minutes& startTime) {
        // Implement Dijkstra
        // Use the schedule to determine the best route based on the start time        
        

        struct Tracker{
            std::chrono::minutes time_traveled;
            bool visited;
            Station* s;
            Tracker* prev = nullptr;
        };
        
       

        //while queue is not empty and current node isn't the end
        std::unordered_map<std::string, Tracker> distances;
        for (const auto& [key, value] : stations) {
            distances[key] = {std::chrono::minutes(MAX_MINUTES), false, &stations[key]};
        }

        //start a priority queue and add starting node
        auto compare = [](const Tracker* a, const Tracker* b) {
            return a->time_traveled > b->time_traveled;
        };
        std::priority_queue<Tracker*, std::vector<Tracker*>, decltype(compare)> pq(compare);


        distances[start] = {std::chrono::minutes(0), true, &stations[start]};
        pq.push(&distances[start]);
        Tracker* curr = &distances[start];

        while(!pq.empty() && end.compare(curr->s->name) != 0){
            //pop from pq
            curr = pq.top();
            pq.pop();

            curr->visited = true;
            
            //iterate through each station connected to current
            for(const auto& connection : this->adjacencyList[curr->s->name]){
                
                Tracker* currCon = &distances.at(connection.to);

                //gets how long we will be waiting for next bus
                auto wait_time = nextWaitTime(connection.schedule, 
                    startTime + curr->time_traveled);
                
                std::cout << "wait time: " << wait_time.count() << std::endl;
                //gets travel time to next stop from here
                //divide speed by 60 since its in km/h and we're measureing minutes
                double travel_distance = this->calculateDistance(curr->s->location, stations[connection.to].location);
                auto travel_time = std::chrono::minutes(static_cast<int>(travel_distance / (connection.average_speed/60)));

                std::cout << "travel time " << travel_time.count() << std::endl;

                auto new_time = curr->time_traveled + wait_time + travel_time;
                if(currCon->time_traveled > new_time){
                    currCon->prev = curr;
                    currCon->time_traveled = new_time;
                    if(!currCon->visited){ 
                        pq.push(currCon);
                    }
                }
                
            }

        }

        //TODO unravel path traveled.
        std::cout << "Unraveling path";
        std::vector<std::string> path;
        std::chrono::minutes travel_time = std::chrono::minutes(0);
        if(curr != nullptr)
            travel_time = curr->time_traveled;

        while(curr != nullptr){
            path.push_back(curr->s->name);
            curr = curr->prev;
        }
        std::reverse(path.begin(), path.end());
        return std::pair(path, travel_time);
    }

    std::pair<std::vector<std::string>, std::chrono::minutes> aStarSearch(const std::string& start, const std::string& end, 
                                      const std::chrono::minutes& startTime) {
        // Implement A* algorithm here
        // Use the schedule to determine the best route based on the start time
        // Consider transfer times between different modes
        return std::pair(std::vector<std::string>(), std::chrono::minutes(0));
    }

public:
    Graph(){
        this->searchAlgo = [this](const std::string& start, const std::string& end, 
                                  const std::chrono::minutes& startTime) {
            return this->Dijkstra(start, end, startTime);
        };
    }

    static std::chrono::minutes getMinutesSinceMidnight(const std::chrono::system_clock::time_point& time_point) {
        auto time_t_point = std::chrono::system_clock::to_time_t(time_point);
        std::tm tm_point = *std::localtime(&time_t_point);
        return std::chrono::minutes(tm_point.tm_hour * 60 + tm_point.tm_min);
    }

    void addStation(const Station& station) {
        stations[station.name] = station;
        adjacencyList[station.name] = std::vector<Connection>();
    }

    void addConnection(const std::string& from, const Connection& connection) {
        adjacencyList[from].push_back(connection);
    }

    double calculateDistance(const Coordinates& c1, const Coordinates& c2) {
        // distance between latitudes
        // and longitudes
        double lat1 = c1.lat;
        double lon1 = c1.lon;
        double lat2 = c2.lat;
        double lon2 = c2.lon;

        double dLat = (lat2 - lat1) * M_PI / 180.0;
        double dLon = (lon2 - lon1) * M_PI / 180.0;
 
        // convert to radians
        lat1 = (lat1) * M_PI / 180.0;
        lat2 = (lat2) * M_PI / 180.0;
 
        // apply formulae
        double a = pow(sin(dLat / 2), 2) + 
                   pow(sin(dLon / 2), 2) * 
                   cos(lat1) * cos(lat2);
        double rad = 6371;
        double c = 2 * asin(sqrt(a));
        return rad * c;
        return 0;
    }

    double getTotalDistance(std::vector<std::string> path){
        int distance_traveled = 0;
        if(path.size() <= 1){
            return 0;
        }
        //always looking towards next node
        for(int i = 0; i < path.size() - 1; i++){
            distance_traveled += calculateDistance(
                stations[path[i]].location, stations[path[i+1]].location
            );
        }

        return distance_traveled;
    }

    std::pair<std::vector<std::string>, std::chrono::minutes> findPath(const std::string& start, const std::string& end, 
                                      const std::chrono::minutes& startTime) {
        return this->searchAlgo(start, end, startTime);
    }
    

    double calculateFare(const std::vector<std::string>& path) {
        // Implement fare calculation based on distance, zones, or fixed rates
        return 0;
    }


    //ASSUMES SCHEDULE IS ALREADY SORTED, USES BINARY SEARCH
    const std::chrono::minutes nextDepartureTime (const std::vector<std::chrono::minutes> schedule, 
        const std::chrono::minutes& currentTime) {
        // Implement logic to find the next departure time based on the current time
        // Return the wait time until the next departure

        //find smallest value that is greater than start time and return it
        auto it = std::upper_bound(schedule.begin(), schedule.end(), currentTime);

        // Check if we found a valid departure time
        if (it != schedule.end()) {
            // Return the next departure time
            return *it;
        } else {
            // Handle case where all departure times are before or equal to currentTime
            //in this case we add a day's worth of minutes to the shortest time
            if (!schedule.empty()) {
                return schedule.front() + std::chrono::hours(24);
            } else {
                // Handle the case where the schedule is empty, return a default large value
                return std::chrono::minutes(std::numeric_limits<int>::max());
            }
        }
    };

    const std::chrono::minutes nextWaitTime (const std::vector<std::chrono::minutes> schedule, 
        const std::chrono::minutes& currentTime) {
        return nextDepartureTime(schedule, currentTime) - currentTime;
    }
};

int main() {
    Graph* transitNetwork = new Graph();

    // Add stations with real coordinates
    transitNetwork->addStation({"Central Station", {40.7128, -74.0060}, true});
    transitNetwork->addStation({"North Station", {40.7589, -73.9851}, false});
    transitNetwork->addStation({"East Station", {40.7549, -73.9840}, true});

    // Add connections with schedules

    transitNetwork->addConnection("Central Station", {"North Station", 60, "bus", {
        get_time_point(8, 0),
        get_time_point(8, 30),
        get_time_point(9, 0)
    }});

    transitNetwork->addConnection("North Station", {"East Station", 45, "train", {
        get_time_point(8, 15),
        get_time_point(8,45),
        get_time_point(9, 15)
    }});


    std::string start, end;
    std::cout << "Enter start station: ";
    std::getline(std::cin, start);
    std::cout << "Enter end station: ";
    std::getline(std::cin, end);


    // Get current time
    auto now = std::chrono::system_clock::now();

    std::pair<std::vector<std::string>, std::chrono::minutes> result = transitNetwork->findPath(start, end, Graph::getMinutesSinceMidnight(now));
    std::vector<std::string> path = result.first;
    std::chrono::minutes travel_time = result.second;

    std::cout << "Path found:\n";
    for (const auto& station : path) {
        std::cout << station << " -> ";
    }
    std::cout << "Destination\n";

    double fare = transitNetwork->calculateFare(path);
    std::cout << "Total fare: $" << fare << std::endl;

    std::cout << "Travel time (minutes) " << travel_time.count() << std::endl;

    std:: cout << "Distance Traveled (km) " << transitNetwork->getTotalDistance(path) << std::endl;
    return 0;
}