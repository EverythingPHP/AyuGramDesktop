#include "PseudoPopup.h"

Ui::PseudoPopup::PseudoPopup(Ui::PopupMenu* menu)
    : _menu(menu)
{
}

not_null<QAction*> Ui::PseudoPopup::addAction(const std::string text, Fn<void()> callback, const style::icon* icon, const style::icon* iconOver)
{
    return _menu->addAction(QString::fromStdString(text), callback, icon, iconOver);
}

not_null<QAction*> Ui::PseudoPopup::addSeparator(const style::MenuSeparator* st)
{
    return _menu->addSeparator(st);
}

void Ui::PseudoPopup::removeAction(int position)
{
    _menu->removeAction(position);
}

void Ui::PseudoPopup::clearActions()
{
    _menu->clearActions();
}

const std::vector<not_null<QAction*>>& Ui::PseudoPopup::actions() const
{
    return _menu->actions();
}

void Ui::PseudoPopup::checkSubmenuShow()
{
    _menu->checkSubmenuShow();
}

void Ui::PseudoPopup::deleteOnHide(bool del)
{
    _menu->deleteOnHide(del);
}

void Ui::PseudoPopup::popup(const QPoint& p)
{
    _menu->popup(p);
}

bool Ui::PseudoPopup::prepareGeometryFor(const QPoint& p)
{
    return _menu->prepareGeometryFor(p);
}

void Ui::PseudoPopup::popupPrepared()
{
    _menu->popupPrepared();
}

void Ui::PseudoPopup::hideMenu(bool fast)
{
    _menu->hideMenu(fast);
}

void Ui::PseudoPopup::setTopShift(int topShift)
{
    _menu->setTopShift(topShift);
}

void Ui::PseudoPopup::setForceWidth(int forceWidth)
{
    _menu->setForceWidth(forceWidth);
}

void Ui::PseudoPopup::setForcedOrigin(PanelAnimation::Origin origin)
{
    _menu->setForcedOrigin(origin);
}

void Ui::PseudoPopup::setForcedVerticalOrigin(Ui::PopupMenu::VerticalOrigin origin)
{
    _menu->setForcedVerticalOrigin(origin);
}

void Ui::PseudoPopup::setAdditionalMenuPadding(QMargins padding, QMargins margins)
{
    _menu->setAdditionalMenuPadding(padding, margins);
}

QMargins Ui::PseudoPopup::additionalMenuPadding() const
{
    return _menu->additionalMenuPadding();
}

QMargins Ui::PseudoPopup::additionalMenuMargins() const
{
    return _menu->additionalMenuMargins();
}

QMargins Ui::PseudoPopup::preparedPadding() const
{
    return _menu->preparedPadding();
}

QMargins Ui::PseudoPopup::preparedMargins() const
{
    return _menu->preparedMargins();
}

bool Ui::PseudoPopup::useTransparency() const
{
    return _menu->useTransparency();
}

int Ui::PseudoPopup::scrollTop() const
{
    return _menu->scrollTop();
}

rpl::producer<int> Ui::PseudoPopup::scrollTopValue() const
{
    return _menu->scrollTopValue();
}
