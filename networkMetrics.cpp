#include <vector>
#include <unordered_set>
#include <queue>
#include <numeric>
#include <thread>
#include <atomic>
#include <iostream>
#include <string>
#include <fstream>
#include <cmath>
#include <tuple>
#include <chrono>
#include <climits>
#include <iomanip>
#include <random>
#include <algorithm>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <sstream>

#include "networkMetrics.h"

using namespace std;

std::tuple<double, double, int> calculatePowerLawCumulative(const vector<vector<int>>& fastGraph) {
    const size_t n = fastGraph.size();
    
    size_t maxDegree = 0;
    
    for (const auto& neighbors : fastGraph) {
        size_t degree = neighbors.size();
        maxDegree = std::max(maxDegree, degree);
    }
    
    vector<int> degreeFrequency(maxDegree + 1, 0);
    
    for (const auto& neighbors : fastGraph) {
        size_t degree = neighbors.size();
        degreeFrequency[degree]++;
    }
    
    // Calculate complementary cumulative distribution P(X >= k)
    vector<double> complementaryCumulativeProbability(maxDegree + 1, 0.0);
    double runningSum = 0.0;
    
    // Start from maxDegree and work backwards
    for (int k = maxDegree; k >= 1; k--) {
        runningSum += static_cast<double>(degreeFrequency[k]) / n;
        complementaryCumulativeProbability[k] = runningSum;
    }

    
    double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumX2 = 0.0, sumY2 = 0.0;
    int m = 0;
    
    const double MIN_PROBABILITY = 1e-10; // Minimum probability threshold
    
    for (size_t k = 1; k < degreeFrequency.size(); k++) {
        if (complementaryCumulativeProbability[k] > MIN_PROBABILITY) {
            double logK = log(static_cast<double>(k));
            double logProb = log(complementaryCumulativeProbability[k]);
            sumX += logK;
            sumY += logProb;
            sumXY += logK * logProb;
            sumX2 += logK * logK;
            sumY2 += logProb * logProb;
            m++;
        } else {
            // If complementaryCumulativeProbability[k] is too small, we can break early
            // since all subsequent values will also be very small (due to the cumulative nature)
            break;
        }
        
        // Additional safety check: if we've processed too many iterations, break
        if (k > 7500) {
            break;
        }
    }
    
    if (m < 2) {
        return {0.0, 0.0, static_cast<int>(maxDegree)};
    }
    
    double slope = (m * sumXY - sumX * sumY) / (m * sumX2 - sumX * sumX);
    
    double r = (m * sumXY - sumX * sumY) / 
               sqrt((m * sumX2 - sumX * sumX) * (m * sumY2 - sumY * sumY));
    
    // For complementary cumulative distribution: slope = -α, so α = 1-slope
    double alpha = 1-slope;

    return {alpha, r, static_cast<int>(maxDegree)};
}

std::tuple<double, double, int> calculatePowerLaw(const vector<vector<int>>& fastGraph) {
    const size_t n = fastGraph.size();
    
    size_t maxDegree = 0;
    
    for (const auto& neighbors : fastGraph) {
        size_t degree = neighbors.size();
        maxDegree = std::max(maxDegree, degree);
    }
    
    vector<int> degreeFrequency(maxDegree + 1, 0);
    
    for (const auto& neighbors : fastGraph) {
        size_t degree = neighbors.size();
        degreeFrequency[degree]++;
    }
    
    vector<double> logDegree;
    vector<double> logProbability;
    
    for (size_t k = 1; k < degreeFrequency.size(); k++) {
        if (degreeFrequency[k] > 0) {
            double probability = static_cast<double>(degreeFrequency[k]) / n;
            logDegree.push_back(log(static_cast<double>(k)));
            logProbability.push_back(log(probability));
        }
    }
    
    if (logDegree.size() < 2) {
        return {0.0, 0.0, static_cast<int>(maxDegree)};
    }
    
    double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumX2 = 0.0, sumY2 = 0.0;
    int m = logDegree.size();
    
    for (int i = 0; i < m; i++) {
        sumX += logDegree[i];
        sumY += logProbability[i];
        sumXY += logDegree[i] * logProbability[i];
        sumX2 += logDegree[i] * logDegree[i];
        sumY2 += logProbability[i] * logProbability[i];
    }
    
    double slope = (m * sumXY - sumX * sumY) / (m * sumX2 - sumX * sumX);
    
    double r = (m * sumXY - sumX * sumY) / 
               sqrt((m * sumX2 - sumX * sumX) * (m * sumY2 - sumY * sumY));
    
    return {-slope, r, static_cast<int>(maxDegree)};
}

std::vector<std::vector<int>> convertToFastGraph(std::vector<std::unordered_set<int>>& graph) {
    const size_t n = graph.size();
    std::vector<std::vector<int>> fastGraph(n);
    
    for (size_t i = 0; i < n; ++i) {
        const size_t degree = graph[i].size();
        fastGraph[i].reserve(degree);
        fastGraph[i].assign(graph[i].begin(), graph[i].end());
    }
    
    return fastGraph;
}

void saveFastGraphToFile(const std::vector<std::vector<int>>& fastGraph, const std::string& executionId) {
    if (executionId.empty()) {
        return;
    }
    
    std::string filename = "checkpoint_" + executionId + "_graph.txt";
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open file " << filename << " for writing graph" << std::endl;
        return;
    }
    
    const size_t n = fastGraph.size();
    file << n << std::endl;
    
    for (size_t i = 0; i < n; ++i) {
        file << fastGraph[i].size();
        for (int neighbor : fastGraph[i]) {
            file << " " << neighbor;
        }
        file << std::endl;
    }
    
    file.close();
    std::cout << "Network graph saved to " << filename << " (for resuming distance calculations)" << std::endl;
}

std::vector<std::vector<int>> loadFastGraphFromFile(const std::string& executionId) {
    std::vector<std::vector<int>> fastGraph;
    
    if (executionId.empty()) {
        return fastGraph;
    }
    
    std::string filename = "checkpoint_" + executionId + "_graph.txt";
    std::ifstream file(filename);
    if (!file.is_open()) {
        return fastGraph;
    }
    
    size_t n;
    file >> n;
    fastGraph.resize(n);
    
    for (size_t i = 0; i < n; ++i) {
        size_t degree;
        file >> degree;
        fastGraph[i].resize(degree);
        for (size_t j = 0; j < degree; ++j) {
            file >> fastGraph[i][j];
        }
    }
    
    file.close();
    std::cout << "Network graph loaded from " << filename << " (" << n << " nodes)" << std::endl;
    return fastGraph;
}

bool loadCheckpointResults(const std::string& executionId, std::vector<double>& nodeResults, size_t& numNodes) {
    nodeResults.clear();
    
    if (executionId.empty()) {
        return false;
    }
    
    std::string filename = "checkpoint_" + executionId + "_results.txt";
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    // Read all node results
    size_t nodeId;
    double avgPathLength;
    size_t maxNodeId = 0;
    std::vector<std::pair<size_t, double>> results;
    
    while (file >> nodeId >> avgPathLength) {
        results.push_back({nodeId, avgPathLength});
        maxNodeId = std::max(maxNodeId, nodeId);
    }
    
    file.close();
    
    if (results.empty()) {
        return false;
    }
    
    // Set numNodes to maxNodeId + 1 (assuming nodes are 0-indexed)
    numNodes = maxNodeId + 1;
    nodeResults.resize(numNodes, -1.0); // -1.0 indicates not computed yet
    
    size_t loadedCount = 0;
    for (const auto& result : results) {
        if (result.first < numNodes) {
            nodeResults[result.first] = result.second;
            loadedCount++;
        }
    }
    
    std::cout << "Loaded checkpoint: " << loadedCount << "/" << numNodes << " nodes completed" << std::endl;
    return true;
}

void saveCompletedNodesList(const std::string& executionId, const std::vector<double>& nodeResults) {
    if (executionId.empty()) {
        return;
    }
    
    std::string filename = "checkpoint_" + executionId + "_completed.txt";
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open file " << filename << " for writing completed nodes list" << std::endl;
        return;
    }
    
    for (size_t i = 0; i < nodeResults.size(); ++i) {
        if (nodeResults[i] >= 0.0) {
            file << i << std::endl;
        }
    }
    
    file.close();
}

std::unordered_set<size_t> loadCompletedNodesList(const std::string& executionId) {
    std::unordered_set<size_t> completedNodes;
    
    if (executionId.empty()) {
        return completedNodes;
    }
    
    std::string filename = "checkpoint_" + executionId + "_completed.txt";
    std::ifstream file(filename);
    if (!file.is_open()) {
        return completedNodes;
    }
    
    size_t nodeId;
    while (file >> nodeId) {
        completedNodes.insert(nodeId);
    }
    
    file.close();
    return completedNodes;
}

void saveNodeResult(const std::string& executionId, size_t nodeId, double avgPathLength) {
    if (executionId.empty()) {
        return;
    }
    
    std::string filename = "checkpoint_" + executionId + "_results.txt";
    std::ofstream file(filename, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open file " << filename << " for writing result" << std::endl;
        return;
    }
    
    file << nodeId << " " << std::fixed << std::setprecision(15) << avgPathLength << std::endl;
    file.close();
    
    // Also update the completed nodes list
    std::string completedFilename = "checkpoint_" + executionId + "_completed.txt";
    std::ofstream completedFile(completedFilename, std::ios::app);
    if (completedFile.is_open()) {
        completedFile << nodeId << std::endl;
        completedFile.close();
    }
}

double calculateClusteringCoefficient(const vector<vector<int>>& fastGraph) {
    const size_t n = fastGraph.size();
    double totalSum = 0.0;

    for (size_t i = 0; i < n; ++i) {
        const size_t degree = fastGraph[i].size();
        if (degree < 2) continue;

        unordered_set<int> neighborSet(fastGraph[i].begin(), fastGraph[i].end());
        
        int connections = 0;
        for (size_t j = 0; j < fastGraph[i].size(); ++j) {
            int neighbor1 = fastGraph[i][j];
            for (size_t k = j + 1; k < fastGraph[i].size(); ++k) {
                int neighbor2 = fastGraph[i][k];
                
                for (int adj : fastGraph[neighbor1]) {
                    if (adj == neighbor2) {
                        connections++;
                        break;
                    }
                }
            }
        }
        
        int maxPossibleConnections = degree * (degree - 1) / 2;
        
        double localCoefficient = connections / static_cast<double>(maxPossibleConnections);
        totalSum += localCoefficient;
    }

    return totalSum / n;
}

double calculateShortestPathLength(const vector<vector<int>>& fastGraph, const std::string& executionId) {
    const size_t n = fastGraph.size();
    double totalSum = 0.0;

    if (n == 0) {
        return 0.0;
    }

    // Save graph if execution ID is provided (for resuming with same network)
    if (!executionId.empty()) {
        // Check if graph file already exists
        std::string graphFilename = "checkpoint_" + executionId + "_graph.txt";
        std::ifstream checkFile(graphFilename);
        if (!checkFile.is_open()) {
            // Graph file doesn't exist, save it
            saveFastGraphToFile(fastGraph, executionId);
        }
        checkFile.close();
    }

    // Load checkpoint results if they exist
    std::vector<double> nodeResults;
    size_t checkpointNumNodes = 0;
    bool hasCheckpoint = false;
    if (!executionId.empty()) {
        hasCheckpoint = loadCheckpointResults(executionId, nodeResults, checkpointNumNodes);
        if (hasCheckpoint) {
            if (checkpointNumNodes != n) {
                std::cerr << "Warning: Checkpoint has " << checkpointNumNodes << " nodes but graph has " << n << " nodes. Ignoring checkpoint." << std::endl;
                hasCheckpoint = false;
                nodeResults.clear();
            } else {
                // Ensure the vector is the right size
                nodeResults.resize(n, -1.0);
            }
        }
    }
    
    if (!hasCheckpoint) {
        nodeResults.resize(n, -1.0); // -1.0 indicates not computed yet
    }

    // Count how many nodes are already computed and sum their results
    size_t completedNodes = 0;
    for (size_t i = 0; i < n; ++i) {
        if (nodeResults[i] >= 0.0) {
            totalSum += nodeResults[i];
            completedNodes++;
        }
    }
    
    if (completedNodes > 0) {
        std::cout << "Resuming: " << completedNodes << "/" << n << " nodes already computed" << std::endl;
    }

    // Save completed nodes list for child processes to use
    if (!executionId.empty() && completedNodes > 0) {
        saveCompletedNodesList(executionId, nodeResults);
    }

    // If all nodes are already computed, return the result
    if (completedNodes == n) {
        std::cout << "All nodes already computed. Returning cached result." << std::endl;
        return totalSum / n;
    }

    const size_t numProcesses = 20; // adapt with the number of cores
    
    pid_t parentPid = getpid();
    std::cout << "Using " << numProcesses << " processes for path length calculation" << std::endl;
    std::cout << "Parent PID: " << parentPid << " (use 'pstree -p " << parentPid << "' to see child processes)" << std::endl;
    
    // Create temporary files for each process to write results
    std::vector<std::string> tempFiles;
    for (size_t i = 0; i < numProcesses; ++i) {
        std::ostringstream oss;
        oss << "/tmp/bfs_result_" << getpid() << "_" << i << ".tmp";
        tempFiles.push_back(oss.str());
    }
    
    // Fork 200 child processes
    for (size_t processId = 0; processId < numProcesses; ++processId) {
        pid_t pid = fork();
        
        if (pid == 0) {
            // Child process
            double processSum = 0.0;
            size_t processCompleted = 0;
            
            // Load completed nodes list to skip already computed nodes
            std::unordered_set<size_t> completedNodes;
            if (!executionId.empty()) {
                completedNodes = loadCompletedNodesList(executionId);
            }
            
            std::vector<char> visited(n, 0);
            std::vector<int> distances(n);
            std::queue<int> q;
            
            // Process nodes where i % 200 == processId
            for (size_t source = processId; source < n; source += numProcesses) {
                // Skip if already computed
                if (completedNodes.count(source) > 0) {
                    continue;
                }
                
                std::fill(visited.begin(), visited.end(), 0);
                std::fill(distances.begin(), distances.end(), 0);
                
                while (!q.empty()) q.pop();
                
                visited[source] = 1;
                q.push(source);
                
                while (!q.empty()) {
                    int current = q.front();
                    q.pop();
                    
                    const std::vector<int>& neighbors = fastGraph[current];
                    const int currentDist = distances[current];
                    
                    for (int neighbor : neighbors) {
                        if (!visited[neighbor]) {
                            visited[neighbor] = 1;
                            distances[neighbor] = currentDist + 1;
                            q.push(neighbor);
                        }
                    }
                }
                
                int distanceSum = 0;
                int validPaths = 0;
                for (size_t i = 0; i < n; ++i) {
                    if (i != source && visited[i]) {
                        distanceSum += distances[i];
                        validPaths++;
                    }
                }
                
                if (validPaths > 0) {
                    double avgPathFromSource = static_cast<double>(distanceSum) / validPaths;
                    processSum += avgPathFromSource;
                    processCompleted++;
                    
                    // Save result immediately if execution ID is provided
                    if (!executionId.empty()) {
                        saveNodeResult(executionId, source, avgPathFromSource);
                    }
                }
            }
            
            // Write result to temporary file
            std::ofstream outFile(tempFiles[processId]);
            if (outFile.is_open()) {
                outFile << std::fixed << std::setprecision(15) << processSum << " " << processCompleted;
                outFile.close();
            }
            
            _exit(0);  // Exit child process
        } else if (pid < 0) {
            // Fork failed
            std::cerr << "Error: Failed to fork process " << processId << std::endl;
            // Clean up already created temp files
            for (size_t i = 0; i < processId; ++i) {
                unlink(tempFiles[i].c_str());
            }
            return 0.0;
        }
        // Parent continues to fork next process
    }
    
    // Parent process: wait for all children to complete
    for (size_t i = 0; i < numProcesses; ++i) {
        int status;
        wait(&status);
    }
    
    // Read results from all temporary files
    size_t newCompletedNodes = 0;
    for (size_t i = 0; i < numProcesses; ++i) {
        std::ifstream inFile(tempFiles[i]);
        if (inFile.is_open()) {
            double processSum = 0.0;
            size_t processCompleted = 0;
            inFile >> processSum >> processCompleted;
            totalSum += processSum;
            newCompletedNodes += processCompleted;
            inFile.close();
        }
        // Clean up temporary file
        unlink(tempFiles[i].c_str());
    }
    
    std::cout << "Completed " << newCompletedNodes << " new nodes. Total: " << (completedNodes + newCompletedNodes) << "/" << n << std::endl;
    
    return totalSum / n;
}

NetworkMetrics calculateAllMetrics(std::vector<std::unordered_set<int>>& graph, const std::string& executionId) {
    NetworkMetrics metrics;
    
    auto fastGraph = convertToFastGraph(graph);
    
    // Count number of edges
    size_t totalDegree = 0;
    for (const auto& neighbors : fastGraph) {
        totalDegree += neighbors.size();
    }
    metrics.numEdges = totalDegree / 2;  // Each edge is counted twice
    
    auto start_time = std::chrono::high_resolution_clock::now();
    auto power_law_result = calculatePowerLaw(fastGraph);
    metrics.powerLawCoefficient = std::get<0>(power_law_result);
    metrics.pearsonR = std::get<1>(power_law_result);
    metrics.maxDegree = std::get<2>(power_law_result);
    
    auto power_law_cumulative_result = calculatePowerLawCumulative(fastGraph);
    metrics.powerLawCumulativeCoefficient = std::get<0>(power_law_cumulative_result);
    metrics.pearsonRCumulative = std::get<1>(power_law_cumulative_result);
    auto end_time = std::chrono::high_resolution_clock::now();
    metrics.powerLawDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    const size_t maxThreads = std::thread::hardware_concurrency();
    const size_t n = fastGraph.size();
    metrics.numThreads = std::min(maxThreads ? maxThreads : 2, n);
    
    start_time = std::chrono::high_resolution_clock::now();
    metrics.avgPathLength = 0.0;
    metrics.avgPathLength = calculateShortestPathLength(fastGraph, executionId);
    end_time = std::chrono::high_resolution_clock::now();
    metrics.avgPathLengthDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    start_time = std::chrono::high_resolution_clock::now();
    metrics.clusteringCoefficient = calculateClusteringCoefficient(fastGraph);
    end_time = std::chrono::high_resolution_clock::now();
    metrics.clusteringCoefficientDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    return metrics;
}
