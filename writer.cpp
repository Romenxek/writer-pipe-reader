#include <windows.h>
#include <iostream>
#include <conio.h>
#include <tchar.h>
#include <string>

int main() {
    const char* message = "Soobshenie s pisatelya";
    TCHAR memName[] = TEXT("SharedMemory");
    LPCTSTR PIPE_NAME = TEXT("\\\\.\\pipe\\pipa");

    HANDLE hPipeCreatingEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, TEXT("PipeCreatingEvent"));
    if (!hPipeCreatingEvent)
    {
        hPipeCreatingEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("PipeCreatingEvent"));
        if (!hPipeCreatingEvent)
        {
            std::cerr << "PipeCreatingEvent in writer failed: " << GetLastError() << std::endl;
            std::cin.get();
            return 1;
        }
    }

    HANDLE hNamedPipe = CreateFile(
        PIPE_NAME,
        GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (hNamedPipe == INVALID_HANDLE_VALUE) // если вернуло не создан канал или занят
    {
        DWORD err = GetLastError();

        if (err == ERROR_FILE_NOT_FOUND)
        {
            std::cout << "Writer is creating the pipe" << std::endl;

            hNamedPipe = CreateNamedPipe(
                PIPE_NAME,                      //имя
                PIPE_ACCESS_DUPLEX,           //
                PIPE_TYPE_MESSAGE | PIPE_WAIT,  //
                1,                              //
                256,                            //
                256,                            //
                0,                              //
                NULL                            //
            );
            if (hNamedPipe == INVALID_HANDLE_VALUE)
            {
                std::cerr << "CreateNamedPipe in writer failed: " << GetLastError() << std::endl;
                CloseHandle(hPipeCreatingEvent);
                std::cin.get();
                return 1;
            }
            SetEvent(hPipeCreatingEvent);
            std::cout << "Pipe was created" << std::endl;
        }
    }
    else 
    {
        std::cout << "Pipe already exists" << std::endl;
        SetEvent(hPipeCreatingEvent);
    }

    WaitForSingleObject(hPipeCreatingEvent, INFINITE);
    std::cout << "Writer connected to the pipe" << std::endl;
    //std::cin.get();

    HANDLE hMessageReadyEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, TEXT("MessageReady"));
    if (!hMessageReadyEvent) {
        hMessageReadyEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("MessageReady"));
        if (!hMessageReadyEvent) {
            std::cerr << "CreateEvent failed: " << GetLastError() << std::endl;
            CloseHandle(hNamedPipe);
            CloseHandle(hPipeCreatingEvent);
            return 1;
        }
    }

    HANDLE hReaderIsFinished = OpenEvent(EVENT_ALL_ACCESS, TRUE, TEXT("ReaderIsReading"));
    if (!hReaderIsFinished) {
        hReaderIsFinished = CreateEvent(NULL, FALSE, TRUE, TEXT("ReaderIsReading"));
        if (!hReaderIsFinished) {
            std::cerr << "CreateEvent failed: " << GetLastError() << std::endl;
            CloseHandle(hMessageReadyEvent);
            CloseHandle(hNamedPipe);
            CloseHandle(hPipeCreatingEvent);
            return 1;
        }
    }

    HANDLE hReaderConnectedEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, TEXT("hReaderConnectedEvent"));
    if (!hReaderConnectedEvent)
    {
        hReaderConnectedEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("hReaderConnectedEvent"));
        if (!hReaderConnectedEvent)
        {
            std::cerr << "ReaderConnectedEvent in writer failed: " << GetLastError() << std::endl;
            std::cin.get();
            CloseHandle(hMessageReadyEvent);
            CloseHandle(hReaderIsFinished);
            CloseHandle(hNamedPipe);
            CloseHandle(hPipeCreatingEvent);
            return 1;
        }
    }

    WaitForSingleObject(hReaderConnectedEvent, INFINITE);
    int i = 0;

    while (true)
    {
        WaitForSingleObject(hReaderIsFinished, INFINITE);

        std::cout << "Writer started writing\n";

        std::string fullMessage = message + std::to_string(i++);
        DWORD bytesWritten;
        WriteFile(hNamedPipe, fullMessage.c_str(), strlen(fullMessage.c_str()) + 1, &bytesWritten, NULL);
        /// ВПЮСАТЬ ВРЕМЯ СЮДА  |
        ///                     |
        ///                     |
        ///                     V
                        Sleep(500);
        std::cout << "Writer finished writing\n";
        SetEvent(hMessageReadyEvent);
    }

    CloseHandle(hMessageReadyEvent);
    CloseHandle(hReaderIsFinished);
    CloseHandle(hNamedPipe);
    CloseHandle(hPipeCreatingEvent);
    CloseHandle(hReaderConnectedEvent);
    return 0;
}