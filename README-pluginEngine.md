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
Plugin defines [AyuPlugin](/Telegram/SourceFiles/AyuPlugin.h) structure, a pointer to that structure, and a function to return it.  <br>
```cpp
AyuPlugin _pluginInfo{
    L"Sample Plugin",
    L"A plugin, demonstrating how AyuGram can accept dynamic-library plugins, and do cool stuff with it.",
    L"AyuSamplePlugin.dll",
    NULL // NULL, because we're re-defining allocated memory space, yk?
}; // end structure

AyuPlugin* _ppluginInfo = &_pluginInfo; // static ptr

EXTERN_DLL_EXPORT AyuPlugin* pluginInfo() { // helper for external access, SHOULD ALWAYS BE pluginInfo
    return _ppluginInfo;
}
```

AyuPlugin struct defines like that, usually: <br>
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
	MemData memData; // MemData struct 
};
// 2/5/2026: bool sharedFiltersEnabled is now deprecated and removed.
```

When a plugin loads, AyuGram process gets that structure and:
- sets MemData
- shows the user whatever plugin is loaded (by name)
- if it fails, it shows them the error code

### DLLMain
DLLMain should return True if the plugin wants to load. It can be set to false, if you want to restrict load, for example.

### Internal Loop
You can define a loop. <br>
`EXTERN_DLL_EXPORT InternalLoop internalLoop() {` <br>
That's it. It's a loop. It'll run in an another loop function in the thread, so you don't have to do: 
```cpp
EXTERN_DLL_EXPORT InternalLoop internalLoop() {
  while (1) {
    // do stuff
  }
}
```
There's nothing much to say about it.

## Hooks

### Rules of hooks
1. Hook should always follow it's defined signature (they're defined in AyuPlugin.h, by the way) and name
2. "Hooks" are stored globally, and accessed in a `for` loop.
3. On BOOL return types of hook functions, the first hook to return TRUE stops the other hook functions from being called
4. There's no "mandatory" hooks. You can't force the plugin to ever, ever, be required to define any kind of hook!
5. Exception to rule 4: Hook chains. Although, currently unused, if there's some hook in the future that may depend on another hook, hook chains are allowed. In that case, ALL of the chained hooks should be present. 

### Shared Filters hook
Must be defined as: `EXTERN_DLL_EXPORT InternalDoFilterHistoryItem doFilterHistoryItem(HistoryItem* HistoryItem)`<br>
Basically, think of it as a `bool doFilterHistoryItem(HistoryItem* historyItem)`, as it always returns a BOOL. <br>
If it RETURNS TRUE, the [filtered function](/Telegram/SourceFiles/ayu/features/filters/filters_controller.cpp#L125C1-L125C51) will filter the matched historyItem. 

### "Online" status hook
If at least one loaded plugin, returns `true`, Ayugram will send an MTProto request for the current user: <br>
`MTPaccount_UpdateStatus(MTP_bool(false))`

Must be defined as: `EXTERN_DLL_EXPORT InternalIsOnline doReturnIsOnline()` <br>
An example usage: <br>
```

DWORD64 GetIdleTimeMs()
{
    LASTINPUTINFO lii;
    lii.cbSize = sizeof(LASTINPUTINFO);

    if (!GetLastInputInfo(&lii))
        return (DWORD64)-1;

    return GetTickCount64() - lii.dwTime;
}

EXTERN_DLL_EXPORT InternalIsOnline doReturnIsOnline()
{
    /*
        Example function that demonstrates Idle online
    */
    const DWORD64 IDLE_LIMIT_MS = 10 * 1000; // 10 seconds

    DWORD64 idleMs = GetIdleTimeMs();
    if (idleMs == (DWORD64)-1)
        return (InternalIsOnline)false;

    return (InternalIsOnline)(idleMs < IDLE_LIMIT_MS);
}
```

### Message hook
It interrupts [this function](/Telegram/SourceFiles/apiwrap.h#L368) in ApiWrap:

```cpp
	void sendMessage(
		MessageToSend &&message,
		std::optional<MsgId> localMessageId = std::nullopt);
```
Must be defined as: `EXTERN_DLL_EXPORT InternalDoPreProcessMessage doPreProcessMessage(char* in, char* out)`  <br>
An example:
			
```cpp
EXTERN_DLL_EXPORT InternalDoPreProcessMessage doPreProcessMessage(char* in, char* out) {
    std::string i(in);
    i=std::regex_replace(i, std::regex("hello"), "replaced");
    strcpy(out, i.c_str());
    out[4096] = '\0';
}
```

## Guides 

### How to load a plugin (via Internal Loader)
1. Put the plugin into "plugins" folder
2. Launch a build of PLEngine AyuGram, and wait.
3. Done, it'll be loaded on startup

### How to load a plugin (via API)
1. Put the plugin into "plugins" folder
2. Launch a build of PLEngine AyuGram, and do an HTTP request to: `http://127.0.0.1:8080/api/runtime/load?name=AyuSamplePlugin.dll` , where name query param is your module name. You may be prompted by a message box, once per every launch. This is normal.
3. Done. Check AyuGram for messageboxes with the debug info.
4. If it crashes, sorry, use a debugger.

## ENV variables
|Variable | Meaning | Default|
| --- | --- | --- |
|`AYUPL_HOST` | Host that'll be used when launching the API | `127.0.0.1`|
|`AYUPL_PORT` | Port that'll be used when launching the API | `8080`|
|`AYUPL_CONSOLE` | If it exists, a separate console on a main thread showing debug info will be shown. | Not Set|

## HTTP API 
PLEngine HTTP API by default runs on 127.0.0.1:8080 (if not redefined).
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


