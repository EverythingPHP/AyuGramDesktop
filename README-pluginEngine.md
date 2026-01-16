# Plugin engine

This is a documentation for AyuGram Plugin engine.<br>
We expect that you're familiar with Building Ayugram Desktop, or with C/C++ in general. <br>
A sample plugin can be found here - https://github.com/MrCheatEugene/AyuSamplePlugin<br>
You need to change the project file to have Ayugram source folders defined accordingly. <br>

## How it works?
It works really simple. 
- We have a DLL, that defines certain functions and exports them
- We load that DLL in a separate thread, and run everything in a separate thread
- We execute those functions, in runtime. While Plugin still has some access to internal functions, and API's.

## How a plugin works?
Plugin defines [AyuPlugin](/Telegram/SourceFiles/AyuPlugin.h) structure, a pointer to that structure, and a function to return it. 
```cpp
AyuPlugin _pluginInfo{
    L"Sample Plugin",
    L"A plugin, demonstrating how AyuGram can accept dynamic-library plugins, and do cool stuff with it.",
    L"AyuSamplePlugin.dll",
    true, // We are defining & using Shadow Filter functions
    NULL // NULL, because we're re-defining allocated memory space, yk?
}; // end structure

AyuPlugin* _ppluginInfo = &_pluginInfo; // static ptr

EXTERN_DLL_EXPORT AyuPlugin* pluginInfo() { // helper for external access, SHOULD ALWAYS BE pluginInfo
    return _ppluginInfo;
}
```

AyuPlugin struct defines like that, usually:
```cpp 
struct MemData
{
	uintptr_t applicationAddr; // an address of Core::App()
	uintptr_t activeUserPtr; // a pointer of a current User - Core::App().activeAccount().session().user()
};

struct AyuPlugin
{
	wchar_t name[128]; // plugin name (will be shown when plugin loads)
	wchar_t description[255]; // plugin description, for future use
	wchar_t moduleName[128]; // module name, for future use
	bool sharedFiltersEnabled; // do we enable shared filters? if yes, we should define a function that processes HistoryItem -> typedef bool (*InternalDoFilterHistoryItem)(HistoryItem *);
	MemData memData; // MemData struct 
};
```

When a plugin loads, AyuGram process gets that structure and:
- sets MemData
- shows the user whatever plugin is loaded (by name)
- if it fails, it shows them the error code

### DLLMain
DLLMain should return True if the plugin wants to load. It can be set to false, if you want to restrict load, for example.

### Shared Filters function
We can define a function like this: `EXTERN_DLL_EXPORT InternalDoFilterHistoryItem doFilterHistoryItem(HistoryItem* HistoryItem)`<br>
That function should be always called `doFilterHistoryItem`, and not something else.<br>
Basically, think of it as a `bool doFilterHistoryItem(HistoryItem* historyItem)`, as it always returns a BOOL. <br>
If it RETURNS TRUE, the [filtered function](/Telegram/SourceFiles/ayu/features/filters/filters_controller.cpp#L125C1-L125C51) will filter the matched historyItem. 

### Internal Loop
You can define a loop.
`EXTERN_DLL_EXPORT InternalLoop internalLoop() {`
That's it. It's a loop. It'll run in an another loop function in the thread, so you don't have to do:
```cpp
EXTERN_DLL_EXPORT InternalLoop internalLoop() {
  while (1) {
    // do stuff
  }
}
```
There's nothing much to say about it.

## How to load a plugin
1. For some reason it worked for me only when I put the DLL in System32. I don't care why right now, it works like that. So put your built DLL in System32
2. Launch a build of PLEngine AyuGram, and do an HTTP request to: `http://127.0.0.1:8080/api/runtime/load?name=AyuSamplePlugin.dll` , where name query param is your module name. You may be prompted by a message box, once per every launch. This is normal.
3. Done. Check AyuGram for messageboxes with the debug info.
4. If it crashes, sorry, use a debugger.

## HTTP API 
PLEngine HTTP API by default runs on 8080 port. 
It can be used for external applications (or plugins) to do stuff, externally. Or even do stuff, without loading a module into AyuGram. 

### Trust elevation 
Whenever you run a certain endpoint, your User-Agent isn't trusted by default.
You may be prompted by a MessageBox from Ayugram. This is made as a simple wall in case something accidentally calls the methods, or tries to maliciously exploit them (||tbh it's not a good security measure, just don't install junk on your PC||)
Trust elevated functions may fail ("ok": "false") if the trust request is rejected.
Each functions can be linked to a trust space: session, runtime, events. Once a user-agent is trusted in a space, it can call the API next time without prompting the user.

### Response
Expect a response of:
```json
{
  "ok": "true/false",
  "error": "human readable error",
  "result_json": "function-specific payload in JSON as string"
}
```

### `/api/ping` 
Ping it. It'll always return "ok": "true"
### `/api/runtime/setPolling?rate=INT`
Trust space "Runtime".
Set polling rate for pointers and updating MemHelper internal structure, where RATE is in miliseconds.
### `/api/runtime/load?name=module.dll`
Trust space "Runtime".
Load a module, see "How to load a plugin" above.
Returns an address of MemHelper.
### `/api/runtime/export`
Trust space "Runtime".
Load MemHelper, start polling pointers. Returns a pointer to the MemHelper internal structure, in case you're managing AyuGram externally.
### `/api/session/get`
Trust space "Session".
Returns a list of loaded Ayugram sessions with structs like:
```cpp
struct SimplifiedTGAccount
{
	int index;
	std::string auth_key;
	int dc;
	long user_id;
	std::string username;
	std::string first_name;
	std::string last_name;
};

void to_json(json &j, const SimplifiedTGAccount &p) {
	j = json{{"index", p.index},
			 {"auth_key", p.auth_key},
			 {"dc", p.dc},
			 {"user_id", p.user_id},
			 {"username", p.username},
			 {"first_name", p.first_name},
			 {"last_name", p.last_name}
	};
}
```


