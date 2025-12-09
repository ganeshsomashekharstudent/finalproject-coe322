#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <queue>
#include <cmath>
using namespace std;

/*
Exercise 50.4:
Apart from the size of the web ("ceiling" in the Web class), some design parameters
include average number of links per page ("links" in create_random_links() in Web class)
and the length of the random walk ("steps" in random_walk in Web class).
Both of these are proportional to how many pages you will visit
*/

// represents a single webpage, storing name and links to other pages
class Page
{
public:
    string name;
    vector<shared_ptr<Page>> pageLinks;
    int index;

    Page(string pageName, int id)
    {
        name = pageName;
        index = id;
    }

    // prints out the name of the webpage
    string as_string()
    {
        return name;
    }

    // adds a link to an exist list of links on a page
    void add_link(shared_ptr<Page> newPage)
    {
        pageLinks.push_back(newPage);
    }

    // returns the page of a given index from the page's list of links
    shared_ptr<Page> click(int index)
    {
        return pageLinks[index];
    }

    // same thing as click() but random
    shared_ptr<Page> random_click()
    {
        if (pageLinks.size() == 0)
        {
            return nullptr;
        }

        int random = rand() % pageLinks.size();
        return pageLinks[random];
    }
};

class ProbabilityDistribution
{
public:
    vector<double> probs;

    // create a probability distribution
    ProbabilityDistribution(int size)
    {
        probs.resize(size, 0.0);
    }

    // access specific element
    double get(int index)
    {
        return probs[index];
    }

    // set random values from 0 to 1
    void set_random()
    {
        for (int i = 0; i < probs.size(); i++)
        {
            probs[i] = rand() / double(RAND_MAX);
        }
    }

    // normalize so the sum of all probabilities is exactly 1 (divide everything by the magnitude)
    void normalization()
    {
        double sum = 0;
        for (int i = 0; i < probs.size(); i++)
        {
            sum += probs[i];
        }
        if (sum > 0)
        {
            for (int i = 0; i < probs.size(); i++)
            {
                probs[i] /= sum;
            }
        }
    }

    // render as a string
    string as_string()
    {
        string s = "[ ";
        for (int i = 0; i < probs.size(); i++)
        {
            s += to_string(probs[i]) + " ";
        }
        s += "]";
        return s;
    }
};

// store a collection of pages and manage creating links between them
class Web
{
public:
    vector<shared_ptr<Page>> pages;

    // creates a certain number of page objects and stores them in a pages vector
    Web(int ceiling)
    {
        for (int i = 0; i < ceiling; i++)
        {
            pages.push_back(make_shared<Page>("Page " + to_string(i), i));
        }
    }

    // adds a specific number of random links from each page to other pages in the web
    void create_random_links(int links)
    {
        for (int i = 0; i < pages.size(); i++)
        {
            for (int j = 0; j < links; j++)
            {
                auto p = pages[rand() % pages.size()];
                pages[i]->add_link(p);
            }
        }
    }

    // jumps from page to page by clicking a random link and then stopping when a page is a dead end
    shared_ptr<Page> random_walk(shared_ptr<Page> current, int steps)
    {
        for (int i = 0; i < steps; i++)
        {
            if (!current)
            {
                cout << "Dead end." << endl;
                return nullptr;
            }

            cout << "Currently at " << current->as_string() << endl;

            current = current->random_click();
        }

        return current;
    }

    // # of pages
    int number_of_pages()
    {
        return pages.size();
    }

    // shows all pages
    vector<shared_ptr<Page>> all_pages()
    {
        return pages;
    }

    // exercise 50.10
    void hypePage(int hypeLinks)
    {

        // created the new hyped page and add it to Web
        auto hype = make_shared<Page>("Hyped Page", pages.size());
        pages.push_back(hype);

        // link n pages to this already hyped page
        for (int i = 0; i < hypeLinks; i++)
        {
            auto random = pages[rand() % (pages.size())];
            random->add_link(hype);
        }
    }

    // exercise 50.8
    // earlier method of globalclick
    /*
    ProbabilityDistribution globalclick(ProbabilityDistribution currentstate)
    {
        ProbabilityDistribution nextstate(currentstate.probs.size()); // all probabilities 0 rn

        for (int i = 0; i < pages.size(); i++){
            double prob = currentstate.get(i); // probability of being at page i

            if (pages[i]->pageLinks.empty()){
                // exercise 50.9
                int random = rand() % pages.size();
                nextstate.probs[random] += prob; // sees if i is a dead end
                // if so, distribute probability to a random page
            }
            else
            {
                // if page not dead end, then see probability of each link on page
                // share is probability split evenly among all linked pages
                double share = prob / (pages[i]->pageLinks.size());
                for (auto linkeds : pages[i]->pageLinks){
                    // loops over linked pages and adds "share" to the probability of each linkedpage
                    nextstate.probs[linkeds->index] += share;
                }
            }
        }

        nextstate.normalization();
        return nextstate;
    }
    */

    // exercise 50.11
    // create a transition matrix
    vector<vector<double>> transitionMatrix(int n)
    {
        vector<vector<double>> Matrix(n, vector<double>(n, 0));

        for (int i = 0; i < n; i++)
        {

            int numLinks = pages[i]->pageLinks.size();
            double prob;

            if (numLinks == 0)
            {
                prob = 1.0;
            }
            else
            {
                prob = 1.0 / numLinks;
            }

            for (auto linked : pages[i]->pageLinks)
            {
                Matrix[i][linked->index] = prob;
            }
        }
        return Matrix;
    }

    vector<double> matrixVectorMultiplication(vector<vector<double>> matrix, vector<double> vect)
    {
        int n = matrix.size();
        vector<double> result(n, 0.0);
        for (int i = 0; i < n; i++)
        {

            for (int j = 0; j < n; j++)
            {
                result[i] += (matrix[i][j] * vect[j]);
            }
        }
        return result;
    }

    vector<double> globalclick(vector<double> currentstate, int numPages)
    {
        vector<vector<double>> Matrix = transitionMatrix(pages.size());              // earlier transition matrix
        vector<double> nextstate = matrixVectorMultiplication(Matrix, currentstate); // earlier Matrix vector multiplication

        // normalize the probability vector
        double sum = 0.0;
        for (double prob : nextstate)
        {
            sum += prob;
        }
        // divide by magnitude
        for (int i = 0; i < nextstate.size(); i++)
        {
            nextstate[i] /= sum;
        }

        return nextstate;
    }
};

// compute shortest paths from the start and the diameter
void shortest(shared_ptr<Page> start, int numPages)
{

    // vector the size of numPages with everything set to -1, because none of the pages have been visited yet
    vector<int> distance(numPages, -1);

    // using queue data structure so we can do a breadth-first search (BFS), bc queues are FIFO
    queue<shared_ptr<Page>> Q;

    // starting page is 0 pages from itself
    distance[start->index] = 0;

    Q.push(start); // put first page on the queue

    // run this while the queue exists
    while (!Q.empty())
    {
        // u is the current page and it gets popped so next up in line can move forward
        auto a = Q.front();
        Q.pop();

        int d = distance[a->index]; // distance it took to reach page u

        // loop through pages u links to, w's a pointer to each page
        for (auto b : a->pageLinks)
        {
            if (distance[b->index] == -1)
            {
                distance[b->index] = d + 1;
                // add w to queue
                Q.push(b);
            }
        }
    }

    // diameter is max steps to reach any reachable page
    int diameter = 0;
    for (int steps : distance)
    {
        if (steps > diameter)
        {
            diameter = steps;
        }
    }

    // print everything
    cout << "Diameter from Page was" << start->index << diameter << endl;
    cout << "Reachable pages:" << endl;
    // loops through all pages by index
    for (int i = 0; i < distance.size(); i++)
    {
        if (distance[i] != -1)
        {
            cout << "  Page " << i << " reachable in " << distance[i] << " steps" << endl;
        }
    }
}

int main()
{
    int nPages = 20;      // total number of pages in the web
    int averageLinks = 2; // average number of links per page
    int steps = 1000;     // number of iterations for PageRank
    int hypeLinks = 5;    // number of pages linking to the hyped page (for exercise 50.10)

    // seeding RNG
    srand(time(nullptr));

    // exercise 50.3
    cout << "--- exercise50.3 ---" << endl;
    Web internet(nPages);
    internet.create_random_links(averageLinks);
    cout << "Web created with " << internet.number_of_pages() << " pages and " << averageLinks << " average links per page." << endl;
    cout << "------------------------------------------" << endl;

    // --- exercise 50.10 ---
    // hyped pages
    cout << "--- Hype Page (Exercise 50.10) ---" << endl;
    internet.hypePage(hypeLinks);
    int totalPages = internet.number_of_pages();
    cout << "Total Amount of pages after hype: " << totalPages << endl;
    cout << "-----------------------------------" << endl;

    // exercise 50.7
    cout << "--- Setting up PageRank ---" << endl;
    // equal distribution (1/n) per page
    vector<double> currentState(totalPages);
    for (int i = 0; i < totalPages; ++i)
    {
        currentState[i] = (1.0 / totalPages);
    }

    cout << "Initial normalized distribution:" << endl;
    // top scores
    int minimum = min(totalPages, 5);
    for (int i = 0; i < minimum; ++i)
    {
        cout << " Page " << i << ": " << currentState[i] << endl;
    }
    cout << "------------------------------------" << endl;

    // 50.8 and 50.11
    cout << "--- 50.8 and 50.11 ---" << endl;
    vector<double> nextState = currentState;

    for (int i = 0; i < steps; ++i)
    {
        // matrix globalclick
        nextState = internet.globalclick(nextState, totalPages);

        cout << "After " << i + 1 << " clicks:" << endl;
        // print probability for hyped page but only if it exists
        if (totalPages > nPages)
        {
            cout << "  Hyped Page (" << (totalPages - 1) << " ): " << nextState[(totalPages - 1)] << endl;
        }

        // print top 5
        cout << "  Top 5 pages:" << endl;
        // find highest probabilities
        vector<pair<double, int>> rankedPages;
        for (int j = 0; j < totalPages; ++j)
        {
            rankedPages.push_back({nextState[j], j});
        }
        sort(rankedPages.rbegin(), rankedPages.rend());

        for (int j = 0; j < min(totalPages, 5); ++j)
        {
            cout << " Page: " << (rankedPages[j].second) << ": " << rankedPages[j].first << endl;
        }

        // convergence check
        double diff = 0.0;
        for (int j = 0; j < totalPages; ++j)
        {
            diff += abs(nextState[j] - currentState[j]);
        }

        if (diff < 1e-6)
        {
            cout << "\nConverged after " << (i + 1) << "steps." << endl;
            break;
        }

        currentState = nextState;
    }
    cout << "------------------------------------------------" << endl;

    // 50.6 ---
    // runs BFS
    cout << "--- 50.6: Shortest path ---" << endl;
    // start at first page
    shared_ptr<Page> startPage = internet.pages[0];
    shortest(startPage, totalPages);
    cout << "--------------------------------------" << endl;

    return 0;
}
