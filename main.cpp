// Include necessary header files
#include <windows.h>
#include <tlhelp32.h>
#include <iostream>

// Function to suspend or resume a process given its process ID
static void SuspendResumeProcess(DWORD dwProcessId, BOOL bResumeThread)
{
    // Take a snapshot of all threads
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

    // If the snapshot is valid
    if (hSnap != INVALID_HANDLE_VALUE)
    {
        // Initialize a THREADENTRY32 structure
        THREADENTRY32 te32;
        te32.dwSize = sizeof(THREADENTRY32);

        // Get the first thread in the snapshot
        if (Thread32First(hSnap, &te32))
        {
            // Loop through all threads in the snapshot
            do
            {
                // If the thread belongs to the process we're interested in
                if (te32.th32OwnerProcessID == dwProcessId)
                {
                    // Open the thread
                    HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);

                    // If the thread handle is valid
                    if (hThread != NULL)
                    {
                        // If we're resuming the thread
                        if (bResumeThread)
                        {
                            // Resume the thread
                            ResumeThread(hThread);
                        }
                        else
                        {
                            // Suspend the thread
                            SuspendThread(hThread);
                        }

                        // Close the thread handle
                        CloseHandle(hThread);
                    }
                }
            } while (Thread32Next(hSnap, &te32)); // Move to the next thread
        }

        // Close the snapshot handle
        CloseHandle(hSnap);
    }
}

// Function to suspend or resume all processes with a given name
static void SuspendResumeProcessByName(BOOL bResumeThread)
{
    // Take a snapshot of all processes
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    // If the snapshot is valid
    if (hSnap != INVALID_HANDLE_VALUE)
    {
        // Initialize a PROCESSENTRY32 structure
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);

        // Get the first process in the snapshot
        if (Process32First(hSnap, &pe32))
        {
            // Loop through all processes in the snapshot
            do
            {
                // If the process name matches the given name (currently hardcoded)
                if (_wcsicmp(pe32.szExeFile, L"Battle.net.exe") == 0 || _wcsicmp(pe32.szExeFile, L"Agent.exe") == 0)
                {
                    // Suspend or resume the process
                    SuspendResumeProcess(pe32.th32ProcessID, bResumeThread);
                }
            } while (Process32Next(hSnap, &pe32)); // Move to the next process
        }

        // Close the snapshot handle
        CloseHandle(hSnap);
    }
}

// Callback function for the window
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Declare nid at the beginning of the switch statement
    NOTIFYICONDATA nid = {};

    switch (msg)
    {
    case WM_CREATE:
        // Add the icon to the system tray when the window is created
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = hwnd;
        nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
        nid.uCallbackMessage = WM_APP;
        //nid.hIcon = (HICON)LoadImage(NULL, L"icon.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
        //nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);

        lstrcpy(nid.szTip, L"Bnet Suspender");
        Shell_NotifyIcon(NIM_ADD, &nid);
        break;

    case WM_DESTROY:
        // Remove the icon from the system tray when the window is destroyed
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = hwnd;
        Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
        break;

    case WM_APP:
        switch (lParam)
        {
        case WM_LBUTTONUP:
            // Left click: suspend processes
            SuspendResumeProcessByName(FALSE);
            //std::cout << "Processes suspended." << std::endl;

            nid.cbSize = sizeof(NOTIFYICONDATA);
            nid.hWnd = hwnd;
            nid.uFlags = NIF_INFO;
            nid.dwInfoFlags = NIIF_INFO;
            lstrcpy(nid.szInfo, L" ");
            lstrcpy(nid.szInfoTitle, L"SUCCESSSSS");

            // Display a notification
            Shell_NotifyIcon(NIM_MODIFY, &nid);

            break;

        case WM_RBUTTONUP:
            // Right click: resume processes
            SuspendResumeProcessByName(TRUE);
            //std::cout << "Processes resumed." << std::endl;

            // Open bnet after right clicking, saves time double clicking the real bnet icon
            ShellExecute(NULL, L"open", L"C:\\Program Files (x86)\\Battle.net\\Battle.net.exe", NULL, NULL, SW_SHOWDEFAULT);
            break;
        }
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int main()
{
	// Hides the console window
    FreeConsole();

    // Specify the names of the processes to suspend or resume
    // No need cuz hardcoded
    //const wchar_t* procName = L"Battle.net.exe"; 
    //const wchar_t* procName2 = L"Agent.exe";



    // Register the window class
    WNDCLASS wc = {}; // Declare a WNDCLASS structure and initialize it to zero
    wc.lpfnWndProc = WndProc; // Set the window procedure function to WndProc
    wc.hInstance = GetModuleHandle(NULL); // Set the instance handle to the current module
    wc.lpszClassName = L"MyClass"; // Set the class name to "MyClass"
    RegisterClass(&wc); // Register the window class with the system

    // This block of code is registering a window class, which is a way for the system to understand what kind of window you’re creating.
    // The WNDCLASS structure contains information about the window class.

    // Create the window
    HWND hwnd = CreateWindow(L"MyClass", L"My Window", 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, GetModuleHandle(NULL), NULL);

    /* This line of code is creating a window. CreateWindow is a function that creates an overlapped, pop - up, or child window with the specified window class, and returns a handle to the new window.
     The parameters are as follows:
        L"MyClass": the name of the window class to use(which was registered earlier).
        L"My Window" : the window name(this will be displayed in the title bar if the window has one).
        0 : the window style(0 means the window has a default style).
        CW_USEDEFAULT : the initial horizontal position of the window.CW_USEDEFAULT means the system selects the default position.
        CW_USEDEFAULT : the initial vertical position of the window.CW_USEDEFAULT means the system selects the default position.
        CW_USEDEFAULT : the initial width of the window.CW_USEDEFAULT means the system selects the default width.
        CW_USEDEFAULT : the initial height of the window.CW_USEDEFAULT means the system selects the default height.
        NULL : the handle to the parent window.NULL means this window has no parent.
        NULL : the handle to the menu for the window.NULL means this window has no menu.
        GetModuleHandle(NULL) : the handle to the instance of the module to be associated with the window.
        NULL : a pointer to a value to be passed to the window through the CREATESTRUCT structure(lpCreateParams member) in the WM_CREATE message.NULL means no additional data is needed. */

    // Run the message loop
    MSG msg = {}; // Declare a MSG structure and initialize it to zero
    while (GetMessage(&msg, NULL, 0, 0)) // Retrieve a message from the message queue
    {
        TranslateMessage(&msg); // Translates virtual-key messages into character messages
        DispatchMessage(&msg); // Dispatches a message to a window procedure
    }

    // This block of code is running the message loop, which is a central part of any Windows application. The message loop retrieves messages from the application’s message queue, translates them,
    // and then dispatches them to the appropriate window procedure for handling.

    // Return 0 to indicate successful execution
    return 0;
}