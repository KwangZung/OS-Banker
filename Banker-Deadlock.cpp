#include <iostream>
#include <fstream>
#include <limits>
using namespace std;

int p_num; //số tiến trình
int r_num; //số tài nguyên

int *u_num; //số đơn vị của mỗi tài nguyên
int *available;
int **toida;
int **allocation;
int **request;
int **need;

/* Quy dinh input
    p_num r_num
    u_num[]
    allocation[]
    toida[]
    request[]
*/

bool isSmallerOrEqual(int* a, int* b) {
    for (int i = 0; i<r_num; i++) {
        if (a[i] > b[i]) return false;
    }
    return true;
}

void addTo(int *work, int* b) {
    for (int i = 0; i < r_num; i++) {
        work[i] = work[i] + b[i];
    }
}
void substractTo(int *a, int *b) {
    for (int i = 0; i < r_num; i++) {
        a[i] = a[i] - b[i];
    }
}

string printArr(int *arr, int len) {
    string str = "(";
    for (int i = 0; i<len; i++) {
        str += to_string(arr[i]);
        if (i < len-1) str += ", ";
    }
    str+=")";
    return str;
}

void initProcessState(string fileName) {
    ifstream file(fileName);
    if (!file) {
        cout << "Open failed\n";
        exit(1);
    }
    file >> p_num >> r_num;
    u_num = new int[r_num];
    available = new int[r_num];
    toida = new int*[p_num];
    allocation = new int*[p_num];
    request = new int*[p_num];
    need = new int*[p_num];
    int allocated_num[r_num];
    for (int i = 0; i < p_num; i++) {
        toida[i] = new int[r_num];
        allocation[i] = new int[r_num];
        request[i] = new int[r_num];
        need[i] = new int[r_num];
    }

    // lấy số đơn vị của mỗi nguồn tài nguyên
    for (int i = 0; i<r_num; i++) {
        file >> u_num[i];
        allocated_num[i] = 0;
    }

    // lấy số đơn vị đang được chiếm bởi tiến trình
    for (int i = 0; i<p_num; i++) {
        for (int j = 0; j<r_num; j++) {
            file >> allocation[i][j];
            if (allocation[i][j] > u_num[j]) cout << "Fault at allocation " << i << " " << j << endl;
            allocated_num[j] += allocation[i][j];
        }
    }

    // lấy số đơn vị mà tiến trình cần
    for (int i = 0; i<p_num; i++) {
        for (int j = 0; j<r_num; j++) {
            file >> toida[i][j];
            if (toida[i][j] > u_num[j]) cout << "Fault at max " << i << " " << j << endl;
            need[i][j] = toida[i][j] - allocation[i][j];
            if (need[i][j] < 0) cout << "Need " << i << " " << j << " is minus\n";
        }
    }

    // lấy số đơn vị tiến trình yêu cầu
    for (int i = 0; i<p_num; i++) {
        for (int j = 0; j<r_num; j++) {
            file >> request[i][j];
            if (request[i][j] > u_num[j]) cout << "Fault at request " << i << " " << j << endl;
        }
    }

    // lấy số đơn vị còn trống
    for (int i = 0; i < r_num; i++) {
        available[i] = u_num[i] - allocated_num[i];
    }
}

void printSystemState() {
    cout << "P\tAllocation\tMax\tAvailable\tNeed\tRequest\n";
    for (int i = 0; i < p_num; i++) {
        cout << i << "\t";
        for (int j = 0; j < r_num; j++) {
            cout << allocation[i][j] << " ";
        }
        cout << "\t";
        for (int j = 0; j < r_num; j++) {
            cout << toida[i][j] << " ";
        }
        cout << "\t";
        for (int j = 0; j < r_num; j++) {
            if (i == 0) cout << available[j] << " ";
        }
        cout << "\t";
        for (int j = 0; j < r_num; j++) {
            cout << need[i][j] << " ";
        }
        cout << "\t";
        for (int j = 0; j < r_num; j++) {
            cout << request[i][j] << " ";
        }
        cout << "\n";
    }
}

// Thuật toán an toàn
bool isSafe(int* safeSequent) {
    // Bước 1
    int work[r_num];
    bool finished[p_num];
    int finished_num = 0; // Số tiến trình đã hoàn thành
    for (int i = 0; i < r_num; i++) {
        work[i] = available[i];
    }
    for (int i =0; i<p_num; i++) {
        finished[i] = false;
    }

    while (finished_num < p_num) {
        // Bước 2
        int i = -1;
        for (int j = 0; j < p_num; j++) {
            if (finished[j] == false && isSmallerOrEqual(need[j], work)) {
                i = j;
                cout << "Need_" << i << " " << printArr(need[i], r_num) << " <= Work " << printArr(work, r_num) << endl;
                break;
            }
        }
        if (i == -1) break;

        // Bước 3
        addTo(work, allocation[i]);
        finished[i] = true;
        safeSequent[finished_num] = i;
        finished_num++;
    }

    // Buoc 4
    return finished_num == p_num;
}

bool banker(int process, int *safeSeq) {
    // Buoc 1
    if (!isSmallerOrEqual(request[process], need[process])) {
        cout << "The process is using more than the number of resources declared\n";
        return false;
    } 
    // Buoc 2
    else if (!isSmallerOrEqual(request[process], available)) {
        cout << "The process must wait because resources are not ready\n";
        return false;
    }
    // Buoc 3
    int *tmpAvailable = available;
    int **tmpAllocation = allocation;
    int **tmpNeed = need;
    substractTo(available, request[process]);
    addTo(allocation[process], request[process]);
    substractTo(need[process], request[process]);

    cout << "If request is approved, then the new state will be:\n";
    printSystemState();
    bool safe = isSafe(safeSeq);
    available = tmpAvailable;
    allocation = tmpAllocation;
    need = tmpNeed;
    return safe;
}

// Thuật toán nhận diện bế tắc
bool isSystemDeadlocking(bool *deadlockProcesses) {
    // Buoc 1
    int work[r_num];
    bool finish[p_num];
    int finished_num = 0;

    for (int i = 0; i < r_num; i++) {
        work[i] = available[i];
    }

    for (int i = 0; i < p_num; i++) {
        if (allocation[i] != 0) finish[i] = false;
        else finish[i] = true;
    }

    while (true) {
        // Buoc 2
        int i = -1;
        for (int j = 0; j < p_num; j++) {
            if (finish[j] == false && isSmallerOrEqual(request[j], work)) {
                i = j;
                cout << "Request_" << i << " " << printArr(request[i], r_num) << " <= Work " << printArr(work, r_num) << endl;
                break;
            }
        }
        if (i == -1) break;

        // Buoc 3
        addTo(work, allocation[i]);
        finish[i] = true;
        finished_num++;
    }
    // Buoc 4
    for (int i = 0; i < p_num; i++) {
        deadlockProcesses[i] = finish[i];
    }
    if (finished_num == p_num) return false;
    return true;
}

int main() {
    string fileName = "testDeadlockDetect.txt";
    initProcessState(fileName);
    printSystemState();

    // Lựa chọn thuật toán
    int selection;
    while (true) {
        cout << "Choose algorithm:\n1. Check whether current state is safe or not (Safety Algorithm)\n"
            << "2. Check request acceptance of a process (Banker)\n"
            << "3. Detect deadlock\n";
        cin >> selection;
        if (cin.fail()) { // Nếu nhập sai (ví dụ: nhập chữ cái)
            cin.clear();  // Xóa trạng thái lỗi
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Loại bỏ dữ liệu sai trong bộ đệm
            cout << "Unrecognizable. Try again!\n";
            continue; // Quay lại đầu vòng lặp
        }
        if (selection == 1 || selection == 2 || selection == 3) break;
        else {
            cout << "Unrecognizable. Try again!\n";
        }
    }

    if (selection == 2) {
        int p;
        while (true) {
            cout << "\nWhich process is requesting? (0 - " << p_num-1 << ")\n";
            cin >> p;
            if (cin.fail()) { // Nếu nhập sai (ví dụ: nhập chữ cái)
                cin.clear();  // Xóa trạng thái lỗi
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Loại bỏ dữ liệu sai trong bộ đệm
                cout << "Unrecognizable. Try again!\n";
                continue; // Quay lại đầu vòng lặp
            }
            if (p >= p_num || p < 0) cout << "\nUnrecognizable. Try again!";
            else break;
        }
        int *safeSeq = new int[p_num];

        if (banker(p, safeSeq)) {
            cout << "\nThe system is safe. Safe sequence: ";
            for (int i = 0; i < p_num; i++)
                cout << safeSeq[i] << " ";
            cout << "\nP" << p  << " request is accepted\n";
        } else {
            cout << "\nP" << p  << " request is not accepted\n";
        }
    } else if (selection == 3) {
        bool *deadlockProcesses = new bool[p_num];
        if (!isSystemDeadlocking(deadlockProcesses)) {
            cout << "\nNo deadlock detected" << endl;
        } else {
            cout << "\nDeadlock detected at process ";
            for (int i = 0; i<p_num; i++) {
                if (deadlockProcesses[i] == false) cout << i << " ";
            }
            cout << "\n";
        }
    } else {
        int safeSeq[p_num];
        if (isSafe(safeSeq)) {
            cout << "\nThe system is safe. Safe sequence: ";
            for (int i = 0; i < p_num; i++)
                cout << safeSeq[i] << " ";
            cout << endl;
        } else {
            cout << "The system is not safe" << endl;
        }
    }
}