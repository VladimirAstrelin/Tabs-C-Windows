#include <windows.h> // Главный заголовочный файл для Windows API. Содержит объявления для большинства функций, структур и констант, необходимых для работы с Windows.
#include <commctrl.h> // Заголовочный файл для общих элементов управления (Common Controls). Он необходим для работы с Tab Control, List View, Tree View и другими современными элементами управления.
#include <tchar.h> // Заголовочный файл для работы с универсальными строками (TCHAR). Это позволяет коду быть совместимым как с ANSI, так и с Unicode (в зависимости от настроек проекта).
#include "resource.h" // Этот файл автоматически генерируется Visual Studio и содержит #define-определения для всех идентификаторов ресурсов (диалогов, кнопок, вкладок и т.д.), которые вы создали в редакторе ресурсов.

// Директива для линковщика. Она указывает, что нужно подключить библиотеку comctl32.lib, в которой находятся функции для работы с общими элементами управления.
#pragma comment(lib, "comctl32.lib")

// Макрос, который определяет количество вкладок в вашем Tab Control. Удобно для циклов.
#define NUM_TABS 5

// Статические глобальные переменные для хранения дескрипторов окон.
// HWND - это дескриптор (идентификатор) окна.
static HWND hTab; // Дескриптор для окна самого Tab Control.
static HWND hTabPages[NUM_TABS] = { 0 }; // Массив дескрипторов для каждого дочернего диалога-страницы.
static int tabIDs[NUM_TABS] = { IDD_TAB1, IDD_TAB2, IDD_TAB3, IDD_TAB4, IDD_TAB5 }; // Массив идентификаторов ресурсов для каждой страницы, определенных в resource.h.

// Прототип функции для оконной процедуры дочерних диалогов-страниц.
// Эта процедура нужна, чтобы обрабатывать сообщения, отправленные элементам, находящимся НА вкладках (например, LTEXT).
INT_PTR CALLBACK TabProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Используем switch для обработки различных сообщений от Windows.
    switch (uMsg) {
        // WM_CTLCOLORSTATIC отправляется родительскому окну перед отрисовкой статического элемента (LTEXT, GROUPBOX).
    case WM_CTLCOLORSTATIC:
    {
        // Получаем контекст устройства для рисования (DC).
        HDC hdcStatic = (HDC)wParam;
        // Устанавливаем режим фона на прозрачный. Это означает, что фон элемента не будет рисоваться.
        SetBkMode(hdcStatic, TRANSPARENT);
        // Возвращаем дескриптор "пустой" (hollow) кисти. Это необходимо, чтобы система знала, что фон не нужно закрашивать.
        return (INT_PTR)GetStockObject(HOLLOW_BRUSH);
    }
    }
    // Если сообщение не было обработано, возвращаем FALSE, чтобы система обработала его по умолчанию.
    return FALSE;
}

// Оконная процедура для главного диалогового окна.
// Здесь обрабатываются сообщения, отправленные самому главному окну и его дочерним элементам.
INT_PTR CALLBACK DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        // Сообщение WM_INITDIALOG отправляется, когда диалоговое окно инициализируется, но ещё не отображено.
    case WM_INITDIALOG:
    {
        // Получаем дескриптор (идентификатор) элемента Tab Control по его ID (IDC_TAB1).
        hTab = GetDlgItem(hwnd, IDC_TAB1);

        // Структура TCITEM содержит информацию об одной вкладке.
        TCITEM tie = { 0 };
        // Маска TCIF_TEXT указывает, что мы будем работать с текстом вкладки.
        tie.mask = TCIF_TEXT;

        // В цикле добавляем все 5 вкладок в Tab Control.
        tie.pszText = _T("ONE"); TabCtrl_InsertItem(hTab, 0, &tie);
        tie.pszText = _T("TWO"); TabCtrl_InsertItem(hTab, 1, &tie);
        tie.pszText = _T("THREE"); TabCtrl_InsertItem(hTab, 2, &tie);
        tie.pszText = _T("FOUR"); TabCtrl_InsertItem(hTab, 3, &tie);
        tie.pszText = _T("FIVE"); TabCtrl_InsertItem(hTab, 4, &tie);

        // Получаем размеры всего Tab Control.
        RECT rc; GetClientRect(hTab, &rc);
        // Функция TabCtrl_AdjustRect корректирует размеры, чтобы получить область только для содержимого вкладок,
        // исключая "ушки" вкладок. FALSE указывает, что мы хотим получить клиентскую область.
        TabCtrl_AdjustRect(hTab, FALSE, &rc);

        // В цикле создаем все дочерние диалоги-страницы.
        for (int i = 0; i < NUM_TABS; i++) {
            // Создаем дочерний диалог из ресурсов.
            // IDD_TAB1, IDD_TAB2 и т.д. определены в resource.h.
            // Ключевое изменение: в качестве оконной процедуры передаем TabProc.
            hTabPages[i] = CreateDialog(
                (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), // Получаем дескриптор текущего модуля
                MAKEINTRESOURCE(tabIDs[i]), // ID ресурса диалога-страницы
                hwnd, // Дескриптор родительского окна
                TabProc); // Оконная процедура для этой вкладки, которая будет обрабатывать сообщения

            // Перемещаем дочерний диалог в вычисленную ранее область rc.
            SetWindowPos(hTabPages[i], NULL, rc.left, rc.top,
                rc.right - rc.left, rc.bottom - rc.top,
                SWP_NOZORDER);

            // Показываем первую вкладку и скрываем остальные.
            ShowWindow(hTabPages[i], i == 0 ? SW_SHOW : SW_HIDE);
        }

        // Принудительно перерисовываем Tab Control. Это нужно, чтобы "ушки" появились сразу, а не только при наведении.
        RedrawWindow(hTab, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

        // Возвращаем TRUE, чтобы Windows установил фокус на первый элемент управления.
        return TRUE;
    }

    // Сообщение WM_NOTIFY отправляется от дочерних элементов управления для уведомления родителя о событиях.
    case WM_NOTIFY:
    {
        // LPNMHDR - это указатель на заголовок структуры уведомления.
        LPNMHDR lpnmhdr = (LPNMHDR)lParam;

        // Проверяем, что уведомление пришло от Tab Control (IDC_TAB1) и что это событие смены вкладки (TCN_SELCHANGE).
        if (lpnmhdr->idFrom == IDC_TAB1 && lpnmhdr->code == TCN_SELCHANGE) {
            // Получаем индекс выбранной вкладки.
            int sel = TabCtrl_GetCurSel(hTab);

            // Цикл для скрытия всех вкладок, кроме выбранной.
            for (int i = 0; i < NUM_TABS; i++) {
                // Если индекс совпадает, показываем вкладку, иначе скрываем.
                ShowWindow(hTabPages[i], i == sel ? SW_SHOW : SW_HIDE);
            }

            // Принудительно перерисовываем Tab Control после смены вкладки.
            RedrawWindow(hTab, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
        }
        break;
    }

    // Сообщение WM_COMMAND отправляется, когда пользователь нажимает на кнопку или выбирает пункт меню.
    case WM_COMMAND:
        // Проверяем, была ли нажата кнопка OK или Cancel.
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            // Закрываем диалоговое окно.
            EndDialog(hwnd, LOWORD(wParam));
            return TRUE;
        }
        break;

        // Сообщение WM_DESTROY отправляется, когда окно закрывается.
    case WM_DESTROY:
    {
        // Отправляем сообщение WM_QUIT, чтобы завершить цикл сообщений и выйти из программы.
        PostQuitMessage(0);
        return TRUE;
    }
    }
    // Если сообщение не было обработано, возвращаем FALSE.
    return FALSE;
}

// Точка входа в программу для Windows-приложений.
int WINAPI wWinMain(
    _In_ HINSTANCE hInstance, // Дескриптор текущего экземпляра приложения.
    _In_opt_ HINSTANCE hPrevInstance, // Дескриптор предыдущего экземпляра (всегда NULL).
    _In_ LPWSTR lpCmdLine, // Командная строка.
    _In_ int nCmdShow) // Режим отображения окна.
{
    // Структура для инициализации общих элементов управления.
    INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_TAB_CLASSES };
    // Инициализируем классы для Tab Control. Это обязательно для корректной работы.
    InitCommonControlsEx(&icc);

    // Создаем и отображаем главное диалоговое окно из ресурсов.
    DialogBoxParam(
        hInstance, // Дескриптор текущего модуля.
        MAKEINTRESOURCE(IDD_DIALOG1), // ID ресурса диалога, который нужно отобразить.
        NULL, // Родительское окно (NULL, так как это главное окно).
        DialogProc, // Указатель на оконную процедуру, которая будет обрабатывать сообщения.
        0); // Дополнительный параметр (не используется).

    // Возвращаем 0, что означает успешное завершение программы.
    return 0;
}

