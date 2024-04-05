#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <unordered_set>
#include <random>
#include <algorithm>

// Present class to represent each present with a unique tag number
class Present
{
public:
    Present(int tag) : tag(tag), next_present(nullptr) {}
    int tag;
    Present *next_present;
};

// ConcurrentLinkedList class implements the linked list operations in a thread-safe manner
class ConcurrentLinkedList
{
public:
    ConcurrentLinkedList() : head(nullptr) {}

    void addPresent(int tag)
    {
        Present *new_present = new Present(tag);
        std::lock_guard<std::mutex> lock(mutex_);
        if (!head || head->tag > tag)
        {
            new_present->next_present = head;
            head = new_present;
        }
        else
        {
            Present *current = head;
            while (current->next_present && current->next_present->tag < tag)
            {
                current = current->next_present;
            }
            new_present->next_present = current->next_present;
            current->next_present = new_present;
        }
    }

    bool removePresent(int tag)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        Present *current = head;
        Present *prev = nullptr;
        while (current)
        {
            if (current->tag == tag)
            {
                if (prev)
                {
                    prev->next_present = current->next_present;
                }
                else
                {
                    head = current->next_present;
                }
                delete current;
                return true;
            }
            prev = current;
            current = current->next_present;
        }
        return false;
    }

    bool searchPresent(int tag)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        Present *current = head;
        while (current)
        {
            if (current->tag == tag)
            {
                return true;
            }
            current = current->next_present;
        }
        return false;
    }

private:
    Present *head;
    std::mutex mutex_;
};

// Function to simulate adding presents to the linked list
void addPresents(ConcurrentLinkedList &linkedList, const std::vector<int> &presentTags)
{
    for (int tag : presentTags)
    {
        linkedList.addPresent(tag);
    }
}

// Function to simulate writing "Thank you" notes and removing presents from the linked list
void writeThankYouNotes(ConcurrentLinkedList &linkedList, const std::vector<int> &presentTags, std::unordered_set<int> &notesWritten)
{
    for (int tag : presentTags)
    {
        if (linkedList.removePresent(tag))
        {
            notesWritten.insert(tag);
        }
    }
}

// Function to simulate searching for presents in the linked list
void searchPresents(ConcurrentLinkedList &linkedList, const std::vector<int> &presentTags, std::unordered_set<int> &presentsFound)
{
    for (int tag : presentTags)
    {
        if (linkedList.searchPresent(tag))
        {
            presentsFound.insert(tag);
        }
    }
}

int main()
{
    constexpr int numPresents = 50000;
    std::vector<int> presentTags(numPresents);
    std::unordered_set<int> notesWritten;
    std::unordered_set<int> presentsFound;

    // Generate random present tags
    std::iota(presentTags.begin(), presentTags.end(), 1);
    std::mt19937 rng(std::random_device{}());
    std::shuffle(presentTags.begin(), presentTags.end(), rng);

    ConcurrentLinkedList linkedList;

    // Create threads for adding presents, writing thank you notes, and searching for presents
    std::vector<std::thread> threads;
    threads.emplace_back(addPresents, std::ref(linkedList), std::cref(presentTags));
    threads.emplace_back(writeThankYouNotes, std::ref(linkedList), std::cref(presentTags), std::ref(notesWritten));
    threads.emplace_back(searchPresents, std::ref(linkedList), std::cref(presentTags), std::ref(presentsFound));

    auto start_time = std::chrono::high_resolution_clock::now();

    for (auto &thread : threads)
    {
        thread.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();

    // Check if all presents are accounted for
    if (notesWritten.size() != numPresents)
    {
        std::cout << "Error: Not all presents have 'Thank you' notes.\n";
    }
    else
    {
        std::cout << "All presents have 'Thank you' notes.\n";
    }

    if (presentsFound.size() != numPresents)
    {
        std::cout << "Error: Not all presents were found in the linked list.\n";
    }
    else
    {
        std::cout << "All presents were found in the linked list.\n";
    }

    auto execution_time = end_time - start_time;
    auto seconds =
        std::chrono::duration_cast<std::chrono::seconds>(execution_time);
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
        execution_time - seconds);

    std::cout << "It took " << seconds.count() << "." << milliseconds.count()
              << " seconds for the program to execute" << std::endl;

    return 0;
}