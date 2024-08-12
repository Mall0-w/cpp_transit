#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <cmath>
#include <chrono>
#include <functional>
#include <queue>

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
    double distance;
    std::string mode; // "bus", "train", "walk", etc.
    std::unordered_map<std::string, std::chrono::minutes> schedule;
};

class Graph {
private:
    std::function<std::vector<std::string>(const std::string&, const std::string&, const std::chrono::system_clock::time_point&)> searchAlgo;
    std::unordered_map<std::string, Station> stations;
    std::unordered_map<std::string, std::vector<Connection>> adjacencyList;

    static std::vector<std::string> Dijkstra(const std::string& start, const std::string& end, 
                                      const std::chrono::system_clock::time_point& startTime) {
        // Implement Dijkstra
        // Use the schedule to determine the best route based on the start time
        // Consider transfer times between different modes

        return std::vector<std::string>();
    }

    static std::vector<std::string> aStarSearch(const std::string& start, const std::string& end, 
                                      const std::chrono::system_clock::time_point& startTime) {
        // Implement A* algorithm here
        // Use the schedule to determine the best route based on the start time
        // Consider transfer times between different modes
        return std::vector<std::string>();
    }

public:
    Graph(){
        this->searchAlgo = Dijkstra;
    }

    void addStation(const Station& station) {
        stations[station.name] = station;
        adjacencyList[station.name] = std::vector<Connection>();
    }

    void addConnection(const std::string& from, const Connection& connection) {
        adjacencyList[from].push_back(connection);
    }

    double calculateDistance(const Coordinates& a, const Coordinates& b) {
        // Haversine formula for distance calculation
        // ... (implement the formula here)
        return 0;
    }

    std::vector<std::string> findPath(const std::string& start, const std::string& end, 
                                      const std::chrono::system_clock::time_point& startTime) {
        return this->searchAlgo(start, end, startTime);
    }
    

    double calculateFare(const std::vector<std::string>& path) {
        // Implement fare calculation based on distance, zones, or fixed rates
        return 0;
    }
};

int main() {
    Graph* transitNetwork = new Graph();

    // Add stations with real coordinates
    transitNetwork->addStation({"Central Station", {40.7128, -74.0060}, true});
    transitNetwork->addStation({"North Station", {40.7589, -73.9851}, false});
    transitNetwork->addStation({"East Station", {40.7549, -73.9840}, true});

    // Add connections with schedules
    transitNetwork->addConnection("Central Station", {"North Station", 5.2, "bus", {
    std::make_pair("08:00", std::chrono::minutes(0)),
    std::make_pair("08:30", std::chrono::minutes(30)),
    std::make_pair("09:00", std::chrono::minutes(60))
}});

transitNetwork->addConnection("North Station", {"East Station", 3.7, "train", {
    std::make_pair("08:15", std::chrono::minutes(0)),
    std::make_pair("08:45", std::chrono::minutes(30)),
    std::make_pair("09:15", std::chrono::minutes(60))
}});


    std::string start, end;
    std::cout << "Enter start station: ";
    std::cin >> start;
    std::cout << "Enter end station: ";
    std::cin >> end;

    // Get current time
    auto now = std::chrono::system_clock::now();

    std::vector<std::string> path = transitNetwork->findPath(start, end, now);

    std::cout << "Path found:\n";
    for (const auto& station : path) {
        std::cout << station << " -> ";
    }
    std::cout << "Destination\n";

    double fare = transitNetwork->calculateFare(path);
    std::cout << "Total fare: $" << fare << std::endl;

    return 0;
}