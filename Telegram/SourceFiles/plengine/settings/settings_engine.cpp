/*
Emil Kh, AKA Pomorgite - t.me/Pomorgite // pmrgt.com
AyuGram Plugin engine, 2026 // t.me/ayuplugg
Follows GNU GPL v3 and Telegram Desktop licensing.

This part of source code is based on Ayugram's sources by Radolyn.
*/

#include "plengine/settings/settings_engine.h"
#include "ayu/ui/settings/ayu_builder.h"
#include "settings/sections/settings_main.h"
#include "plengine/PLESettings.h"
#include "lang_auto.h"
#include "core/version.h"
#include "plengine/settings/settings_main.h"
#include "ayu/ui/settings/settings_main.h"
#include "settings/settings_builder.h"
#include "settings/settings_common.h"
#include "styles/style_layers.h"
#include "styles/style_menu_icons.h"
#include "styles/style_ayu_settings.h"
#include "styles/style_settings.h"
#include "ui/painter.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/labels.h"
#include "ui/wrap/vertical_layout.h"
#include "window/window_session_controller.h"
#include "window/window_session_controller_link_info.h"

#include <QDesktopServices>
#include <ShlObj.h>

namespace Settings {

using namespace Builder;
using namespace Settings::AyuBuilder;
PLESettings* settings = PLESettings::getInstance();

namespace {

std::string OpenFileDialogue() {
	IFileOpenDialog* pfd;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&pfd));
	if (SUCCEEDED(hr))
	{
		DWORD dwOptions;
		hr = pfd->GetOptions(&dwOptions);
		if (SUCCEEDED(hr))
		{
			hr = pfd->SetOptions(dwOptions | FOS_ALLOWMULTISELECT);
		}
		if (SUCCEEDED(hr))
		{
			hr = pfd->Show(NULL);
			if (SUCCEEDED(hr))
			{
				IShellItemArray* psiaResults;
				hr = pfd->GetResults(&psiaResults);
				if (SUCCEEDED(hr))
				{
					DWORD a;
					psiaResults->GetCount(&a);
					if (a != 0) {
						IShellItem* res;
						psiaResults->GetItemAt(0, &res);
						PWSTR pszFilePath = nullptr;
						hr = res->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
						if (SUCCEEDED(hr)) {
							char fn[1025];
							wcstombs(fn, pszFilePath, 1024);
							return std::string(fn);
						}
					}

					psiaResults->Release();
				}
			}
		}
		pfd->Release();
	}
	return std::string();
}
	
void BuildCategories(SectionBuilder &builder, AyuSectionBuilder &ayu) {
	builder.addSkip();
	ayu.addToggle({
		.id = u"ple/enableConsole"_q,
		.title = rpl::single(QString("Enable console(applies at restart)")),
		.getter = []() {return settings->consoleEnabled(); },
		.setter = [](bool val) {settings->setConsoleEnabled(val); }
		});
	builder.addSkip();
	ayu.addToggle({
		.id = u"ple/apiEnabled"_q,
		.title = rpl::single(QString("Enable HTTP API (127.0.0.1:8081)")),
		.getter = []() {return settings->apiEnabled(); },
		.setter = [](bool val) {settings->setapiEnabled(val); }
		});
	builder.addSkip();
	ayu.addToggle({
		.id = u"ple/apiSecEnabled"_q,
		.title = rpl::single(QString("Disable security features in HTTP API")),
		.getter = []() {return settings->apiSecEnabled(); },
		.setter = [](bool val) {settings->setapiSecEnabled(val); }
		});

	builder.addSkip();
	builder.addDividerText(rpl::single(QString("HTTPS requires valid SSL certificates in PEM to be loaded.\nPLEngine does ot validate SSL certificate data.\nThe UI updates the state of the certificates on startup: that's why after selecting you may see old cert's.\nRestart to apply any changes.")));
	builder.addSkip();
	builder.addButton({
		.id = u"ple/apiCert"_q,
		.title = rpl::single(QString::fromStdString(std::string("HTTPS Certificate: "+ settings->apiCert()) )),
		.onClick = ([=] {
			settings->setCert(OpenFileDialogue());
		})
		});
	builder.addSkip();
	builder.addButton({
		.id = u"ple/apiKey"_q,
		.title = rpl::single(QString::fromStdString(std::string("HTTPS Key: " + settings->apiKey()))),
		.onClick = ([=] {
			settings->setKey(OpenFileDialogue());
		})
		});
	builder.addSkip();
}


const auto kMeta = BuildHelper({
	.id = PLEEngine::Id(),
	.parentId = MainId(),
	.title = &tr::ayu_AyuPreferences,
	.icon = &st::menuIconPremium,
}, [](SectionBuilder &builder) {
	auto ayu = AyuSectionBuilder(builder);
	BuildCategories(builder, ayu);
});

} // namespace

rpl::producer<QString> PLEEngine::title() {
	return rpl::single(QString(""));
}

PLEEngine::PLEEngine(
	QWidget *parent,
	not_null<Window::SessionController*> controller)
: Section(parent, controller) {
	setupContent();
}

void PLEEngine::setupContent() {
	const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
	build(content, kMeta.build);
	Ui::ResizeFitChild(this, content);
}

Type PLEEngineId() {
	return PLEEngine::Id();
}

} // namespace Settings
