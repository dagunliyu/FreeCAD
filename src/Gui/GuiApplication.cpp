/***************************************************************************
 *   Copyright (c) 2015 Werner Mayer <wmayer[at]users.sourceforge.net>     *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/


#include "PreCompiled.h"

#ifndef _PreComp_
# include <sstream>
# include <QAbstractSpinBox>
# include <QByteArray>
# include <QComboBox>
# include <QDataStream>
# include <QFileInfo>
# include <QFileOpenEvent>
# include <QSessionManager>
# include <QTimer>
#endif

#include <QLocalServer>
#include <QLocalSocket>

#if defined(Q_OS_UNIX)
# include <sys/types.h>
# include <ctime>
# include <unistd.h>
#endif

#include <App/Application.h>
#include <Base/Console.h>
#include <Base/Exception.h>

#include "GuiApplication.h"
#include "Application.h"
#include "MainWindow.h"
#include "SpaceballEvent.h"


using namespace Gui;

GUIApplication::GUIApplication(int & argc, char ** argv)
    : GUIApplicationNativeEventAware(argc, argv)
{
    connect(this, &GUIApplication::commitDataRequest,
            this, &GUIApplication::commitData, Qt::DirectConnection);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    setFallbackSessionManagementEnabled(false);
#endif
}

GUIApplication::~GUIApplication()
{
}

const char* getEventName(QEvent::Type type)
{
    switch (type)
    {
    case QEvent::Type::None: return "None";	break;//
    case QEvent::Type::Timer: return "Timer";		break;// timer event
    case QEvent::Type::MouseButtonPress: return "MouseButtonPress";	break;// = 2,                   // mouse button pressed
    case QEvent::Type::MouseButtonRelease: return "MouseButtonRelease";	break;// = 3,                 // mouse button released
    case QEvent::Type::MouseButtonDblClick: return "MouseButtonDblClick";	break;// = 4,                // mouse button double click
    case QEvent::Type::MouseMove: return "MouseMove";	break;// = 5,                          // mouse move
    case QEvent::Type::KeyPress: return "KeyPress";	break;//= 6,                           // key pressed
    case QEvent::Type::KeyRelease: return "KeyRelease";	break;// = 7,                         // key released
    case QEvent::Type::FocusIn: return "FocusIn";	break;// = 8,                            // keyboard focus received
    case QEvent::Type::FocusOut: return "FocusOut";	break;// = 9,                           // keyboard focus lost
    case QEvent::Type::FocusAboutToChange: return "FocusAboutToChange";	break;// = 23,                // keyboard focus is about to be lost
    case QEvent::Type::Enter: return "Enter";	break;// = 10,                             // mouse enters widget
    case QEvent::Type::Leave: return "Leave";	break;// = 11,                             // mouse leaves widget
    case QEvent::Type::Paint: return "Paint";	break;// = 12,                             // paint widget
    case QEvent::Type::Move: return "Move: ";	break;//= 13,                              // move widget
    case QEvent::Type::Resize: return "Resize";	break;//= 14,                            // resize widget
    case QEvent::Type::Create: return "Create";	break;//= 15,                            // after widget creation
    case QEvent::Type::Destroy: return "Destroy";	break;// = 16,                           // during widget destruction
    case QEvent::Type::Show: return "Show";	break;//= 17,                              // widget is shown
    case QEvent::Type::Hide: return "Hide";	break;//= 18,                              // widget is hidden
    case QEvent::Type::Close: return "Close";	break;//= 19,                             // request to close widget
    case QEvent::Type::Quit: return "Quit";	break;//= 20,                              // request to quit application
    case QEvent::Type::ParentChange: return "ParentChange";	break;// = 21,                      // widget has been reparented
    case QEvent::Type::ParentAboutToChange: return "ParentAboutToChange";	break;//= 131,              // sent just before the parent change is done
    case QEvent::Type::ThreadChange: return "ThreadChange";	break;//= 22,                      // object has changed threads
    case QEvent::Type::WindowActivate: return "WindowActivate";	break;// = 24,                    // window was activated
    case QEvent::Type::WindowDeactivate: return "WindowDeactivate";	break;//= 25,                  // window was deactivated
    case QEvent::Type::ShowToParent: return "ShowToParent";	break;// = 26,                      // widget is shown to parent
    case QEvent::Type::HideToParent: return "HideToParent";	break;// = 27,                      // widget is hidden to parent
    case QEvent::Type::Wheel: return "Wheel";	break;// = 31,                             // wheel event
    case QEvent::Type::WindowTitleChange: return "WindowTitleChange";	break;// = 33,                 // window title changed
    case QEvent::Type::WindowIconChange: return "WindowIconChange";	break;// = 34,                  // icon changed
    case QEvent::Type::ApplicationWindowIconChange: return "ApplicationWindowIconChange";	break;//= 35,       // application icon changed
    case QEvent::Type::ApplicationFontChange: return "ApplicationFontChange";	break;//= 36,             // application font changed
    case QEvent::Type::ApplicationLayoutDirectionChange: return "ApplicationLayoutDirectionChange";	break;// = 37,  // application layout direction changed
    case QEvent::Type::ApplicationPaletteChange: return "ApplicationPaletteChange";	break;//= 38,          // application palette changed
    case QEvent::Type::PaletteChange:						return "PaletteChange";	break;//= 39,                     // widget palette changed
    case QEvent::Type::Clipboard:							return "Clipboard";	break;// = 40,                         // internal clipboard event
    case QEvent::Type::Speech:								return "Speech";	break;// = 42,                            // reserved for speech input
    case QEvent::Type::MetaCall:							return "MetaCall";	break;//= 43,                         // meta call event
    case QEvent::Type::SockAct:								return "SockAct";	break;//= 50,                           // socket activation
    case QEvent::Type::WinEventAct:							return "WinEventAct";	break;//= 132,                      // win event activation
    case QEvent::Type::DeferredDelete:						return "DeferredDelete";	break;// = 52,                    // deferred delete event
    case QEvent::Type::DragEnter:							return "DragEnter";	break;// = 60,                         // drag moves into widget
    case QEvent::Type::DragMove:							return "DragMove";	break;// = 61,                          // drag moves in widget
    case QEvent::Type::DragLeave:							return "DragLeave";	break;//= 62,                         // drag leaves or is cancelled
    case QEvent::Type::Drop:								return "Drop";	break;//= 63,                              // actual drop
    case QEvent::Type::DragResponse:						return "DragResponse";	break;//= 64,                      // drag accepted/rejected
    case QEvent::Type::ChildAdded:							return "ChildAdded";	break;// = 68,                        // new child widget
    case QEvent::Type::ChildPolished:						return "ChildPolished";	break;//= 69,                     // polished child widget
    case QEvent::Type::ChildRemoved:						return "ChildRemoved";	break;// = 71,                      // deleted child widget
    case QEvent::Type::ShowWindowRequest:					return "ShowWindowRequest";	break;//= 73,                 // widget's window should be mapped
    case QEvent::Type::PolishRequest:						return "PolishRequest";	break;//= 74,                     // widget should be polished
    case QEvent::Type::Polish:								return "Polish";	break;//= 75,                            // widget is polished
    case QEvent::Type::LayoutRequest:						return "LayoutRequest";	break;//= 76,                     // widget should be relayouted
    case QEvent::Type::UpdateRequest:						return "UpdateRequest";	break;//= 77,                     // widget should be repainted
    case QEvent::Type::UpdateLater:							return "UpdateLater";	break;//= 78,                       // request update() later
    case QEvent::Type::EmbeddingControl:					return "EmbeddingControl";	break;//= 79,                  // ActiveX embedding
    case QEvent::Type::ActivateControl:						return "ActivateControl";	break;// = 80,                   // ActiveX activation
    case QEvent::Type::DeactivateControl:					return "DeactivateControl";	break;//= 81,                 // ActiveX deactivation
    case QEvent::Type::ContextMenu:							return "ContextMenu";	break;//= 82,                       // context popup menu
    case QEvent::Type::InputMethod:							return "InputMethod";	break;//= 83,                       // input method
    case QEvent::Type::TabletMove:							return "TabletMove";	break;//= 87,                        // Wacom tablet event
    case QEvent::Type::LocaleChange:						return "LocaleChange";	break;//= 88,                      // the system locale changed
    case QEvent::Type::LanguageChange:						return "LanguageChange";	break;//= 89,                    // the application language changed
    case QEvent::Type::LayoutDirectionChange:				return "LayoutDirectionChange";	break;//= 90,             // the layout direction changed
    case QEvent::Type::Style:								return "Style";	break;//= 91,                             // internal style event
    case QEvent::Type::TabletPress:							return "TabletPress";	break;// = 92,                       // tablet press
    case QEvent::Type::TabletRelease:						return "TabletRelease";	break;//= 93,                     // tablet release
    case QEvent::Type::OkRequest:							return "OkRequest";	break;//= 94,                         // CE (Ok) button pressed
    case QEvent::Type::HelpRequest:							return "HelpRequest";	break;//= 95,                       // CE (?)  button pressed
    case QEvent::Type::IconDrag:							return "IconDrag";	break;// = 96,                          // proxy icon dragged
    case QEvent::Type::FontChange:							return "FontChange";	break;//= 97,                        // font has changed
    case QEvent::Type::EnabledChange:						return "EnabledChange";	break;//= 98,                     // enabled state has changed
    case QEvent::Type::ActivationChange:					return "ActivationChange";	break;//= 99,                  // window activation has changed
    case QEvent::Type::StyleChange:							return "StyleChange";	break;// = 100,                      // style has changed
    case QEvent::Type::IconTextChange:						return "IconTextChange";	break;//= 101,                   // icon text has changed.  Deprecated.
    case QEvent::Type::ModifiedChange:						return "ModifiedChange";	break;//= 102,                   // modified state has changed
    case QEvent::Type::MouseTrackingChange:					return "MouseTrackingChange";	break;//= 109,              // mouse tracking state has changed
    case QEvent::Type::WindowBlocked:						return "WindowBlocked";	break;//= 103,                    // window is about to be blocked modally
    case QEvent::Type::WindowUnblocked:						return "WindowUnblocked";	break;//= 104,                  // windows modal blocking has ended
    case QEvent::Type::WindowStateChange:					return "WindowStateChange";	break;//= 105,
    case QEvent::Type::ReadOnlyChange:						return "ReadOnlyChange";	break;//= 106,                   // readonly state has changed
    case QEvent::Type::ToolTip:								return "ToolTip";	break;//= 110,
    case QEvent::Type::WhatsThis:							return "WhatsThis";	break;//= 111,
    case QEvent::Type::StatusTip:							return "StatusTip";	break;//= 112,
    case QEvent::Type::ActionChanged:						return "ActionChanged";	break;//= 113,
    case QEvent::Type::ActionAdded:							return "ActionAdded";	break;//= 114,
    case QEvent::Type::ActionRemoved:						return "ActionRemoved";	break;//= 115,
    case QEvent::Type::FileOpen:							return "FileOpen";	break;// = 116,                         // file open request
    case QEvent::Type::Shortcut:							return "Shortcut";	break;//= 117,                         // shortcut triggered
    case QEvent::Type::ShortcutOverride:					return "ShortcutOverride";	break;//= 51,                  // shortcut override request
    case QEvent::Type::WhatsThisClicked:					return "WhatsThisClicked";	break;//= 118,
    case QEvent::Type::ToolBarChange:						return "ToolBarChange";	break;// = 120,                    // toolbar visibility toggled
    case QEvent::Type::ApplicationActivate:					return "ApplicationActivate";	break;//= 121,              // deprecated. Use ApplicationStateChange instead.
    case QEvent::Type::ApplicationDeactivate:				return "ApplicationDeactivate";	break;//= 122,            // deprecated. Use ApplicationStateChange instead.
    case QEvent::Type::QueryWhatsThis:						return "QueryWhatsThis";	break;//= 123,                   // query what's this widget help
    case QEvent::Type::EnterWhatsThisMode:					return "EnterWhatsThisMode";	break;//= 124,
    case QEvent::Type::LeaveWhatsThisMode:					return "LeaveWhatsThisMode";	break;//= 125,
    case QEvent::Type::ZOrderChange:						return "ZOrderChange";	break;// = 126,                     // child widget has had its z-order changed
    case QEvent::Type::HoverEnter:							return "HoverEnter";	break;//= 127,                       // mouse cursor enters a hover widget
    case QEvent::Type::HoverLeave:							return "HoverLeave";	break;// = 128,                       // mouse cursor leaves a hover widget
    case QEvent::Type::HoverMove:							return "HoverMove";	break;//= 129,                        // mouse cursor move inside a hover widget
    case QEvent::Type::AcceptDropsChange:					return "AcceptDropsChange";	break;//= 152,
    case QEvent::Type::ZeroTimerEvent:						return "ZeroTimerEvent";	break;//= 154,                   // Used for Windows Zero timer events
    case QEvent::Type::GraphicsSceneMouseMove:				return "GraphicsSceneMouseMove";	break;//= 155,           // GraphicsView
    case QEvent::Type::GraphicsSceneMousePress:				return "GraphicsSceneMousePress";	break;//= 156,
    case QEvent::Type::GraphicsSceneMouseRelease:			return "GraphicsSceneMouseRelease";	break;//= 157,
    case QEvent::Type::GraphicsSceneMouseDoubleClick:		return "GraphicsSceneMouseDoubleClick";	break;//= 158,
    case QEvent::Type::GraphicsSceneContextMenu:			return "GraphicsSceneContextMenu";	break;//= 159,
    case QEvent::Type::GraphicsSceneHoverEnter:				return "GraphicsSceneHoverEnter";	break;//= 160,
    case QEvent::Type::GraphicsSceneHoverMove:				return "GraphicsSceneHoverMove";	break;//= 161,
    case QEvent::Type::GraphicsSceneHoverLeave:				return "GraphicsSceneHoverLeave";	break;//= 162,
    case QEvent::Type::GraphicsSceneHelp:					return "GraphicsSceneHelp";	break;//= 163,
    case QEvent::Type::GraphicsSceneDragEnter:				return "GraphicsSceneDragEnter";	break;//= 164,
    case QEvent::Type::GraphicsSceneDragMove:				return "GraphicsSceneDragMove";	break;//= 165,
    case QEvent::Type::GraphicsSceneDragLeave:				return "GraphicsSceneDragLeave";	break;//= 166,
    case QEvent::Type::GraphicsSceneDrop:					return "GraphicsSceneDrop";	break;//= 167,
    case QEvent::Type::GraphicsSceneWheel:					return "GraphicsSceneWheel";	break;//= 168,
    case QEvent::Type::KeyboardLayoutChange:				return "KeyboardLayoutChange";	break;// = 169,             // keyboard layout chang
    case QEvent::Type::DynamicPropertyChange:				return "DynamicPropertyChange";	break;//= 170,            // A dynamic property was changed through setProperty/property
    case QEvent::Type::TabletEnterProximity:				return "TabletEnterProximity";	break;//= 171,
    case QEvent::Type::TabletLeaveProximity:				return "TabletLeaveProximity";	break;//= 172,
    case QEvent::Type::NonClientAreaMouseMove:				return "NonClientAreaMouseMove";	break;//= 173,
    case QEvent::Type::NonClientAreaMouseButtonPress:		return "NonClientAreaMouseButtonPress";	break;//= 174,
    case QEvent::Type::NonClientAreaMouseButtonRelease:		return "NonClientAreaMouseButtonRelease";	break;//= 175,
    case QEvent::Type::NonClientAreaMouseButtonDblClick:	return "NonClientAreaMouseButtonDblClick";	break;//= 176,
    case QEvent::Type::MacSizeChange:						return "MacSizeChange";	break;// = 177,                    // when the Qt::WA_Mac{Normal,Small,Mini}Size changes
    case QEvent::Type::ContentsRectChange:					return "ContentsRectChange";	break;//= 178,               // sent by QWidget::setContentsMargins (internal)
    case QEvent::Type::MacGLWindowChange:					return "MacGLWindowChange";	break;// = 179,                // Internal! the window of the GLWidget has changed
    case QEvent::Type::FutureCallOut:						return "FutureCallOut";	break;// = 180,
    case QEvent::Type::GraphicsSceneResize:					return "GraphicsSceneResize";	break;// = 181,
    case QEvent::Type::GraphicsSceneMove:					return "GraphicsSceneMove";	break;//= 182,
    case QEvent::Type::CursorChange:						return "CursorChange";	break;//= 183,
    case QEvent::Type::ToolTipChange:						return "ToolTipChange";	break;//= 184,
    case QEvent::Type::NetworkReplyUpdated:					return "NetworkReplyUpdated";	break;// = 185,              // Internal for QNetworkReply
    case QEvent::Type::GrabMouse:							return "GrabMouse";	break;//= 186,
    case QEvent::Type::UngrabMouse:							return "UngrabMouse";	break;// = 187,
    case QEvent::Type::GrabKeyboard:						return "GrabKeyboard";	break;//= 188,
    case QEvent::Type::UngrabKeyboard:						return "UngrabKeyboard";	break;//= 189,
    case QEvent::Type::MacGLClearDrawable:					return "MacGLClearDrawable";	break;//= 191,               // Internal Cocoa, the window has changed, so we must clear
    case QEvent::Type::StateMachineSignal:					return "StateMachineSignal";	break;//= 192,
    case QEvent::Type::StateMachineWrapped:					return "StateMachineWrapped";	break;//= 193,
    case QEvent::Type::TouchBegin:							return "TouchBegin";	break;//= 194,
    case QEvent::Type::TouchUpdate: 						return "TouchUpdate";	break;// = 195,
    case QEvent::Type::TouchEnd: 							return "TouchEnd";	break;//= 196,
    case QEvent::Type::NativeGesture: 						return "NativeGesture";	break;//= 197,                    // QtGui native gesture#endif
    case QEvent::Type::RequestSoftwareInputPanel: 			return "RequestSoftwareInputPanel";	break;//= 199,
    case QEvent::Type::CloseSoftwareInputPanel: 			return "CloseSoftwareInputPanel";	break;// = 200,
    case QEvent::Type::WinIdChange: 						return "WinIdChange";	break;//= 203,
    case QEvent::Type::Gesture: 							return "Gesture";	break;//= 198,
    case QEvent::Type::GestureOverride: 					return "GestureOverride";	break;//= 202,
    case QEvent::Type::ScrollPrepare: 						return "ScrollPrepare";	break;//= 204,
    case QEvent::Type::Scroll: 								return "Scroll";	break;//= 205,
    case QEvent::Type::Expose: 								return "Expose";	break;//= 206,
    case QEvent::Type::InputMethodQuery: 					return "InputMethodQuery";	break;//= 207,
    case QEvent::Type::OrientationChange: 					return "OrientationChange";	break;//= 208,                // Screen orientation has changed
    case QEvent::Type::TouchCancel: 						return "TouchCancel";	break;//= 209,
    case QEvent::Type::ThemeChange: 						return "ThemeChange";	break;// = 210,
    case QEvent::Type::SockClose: 							return "SockClose";	break;//= 211,                        // socket closed
    case QEvent::Type::PlatformPanel: 						return "PlatformPanel";	break;//= 212,
    case QEvent::Type::StyleAnimationUpdate: 				return "StyleAnimationUpdate";	break;//= 213,             // style animation target should be updated
    case QEvent::Type::ApplicationStateChange: 				return "ApplicationStateChange";	break;//= 214,
    case QEvent::Type::WindowChangeInternal: 				return "WindowChangeInternal";	break;//= 215,             // internal for QQuickWidget
    case QEvent::Type::ScreenChangeInternal: 				return "ScreenChangeInternal";	break;//= 216,
    case QEvent::Type::PlatformSurface: 					return "PlatformSurface";	break;//= 217,                  // Platform surface created or about to be destroyed
    case QEvent::Type::Pointer: 							return "Pointer";	break;// = 218,                          // QQuickPointerEvent; ### Qt 6: QPointerEvent
    case QEvent::Type::TabletTrackingChange: 				return "TabletTrackingChange";	break;// = 219,             // tablet tracking state has changed
    case QEvent::Type::User: 								return "User";	break;//= 1000,                            // first user event id
    case QEvent::Type::MaxUser: 							return "MaxUser";	break;//= 65535                         // last user event id
    default:
        return "NOTFIND!!";
        break;
    }
    /*
      If you get a strange compiler error on the line with None,
      it's probably because you're also including X11 headers,
      which #define the symbol None. Put the X11 includes after
      the Qt includes to solve this problem.
    */
}

bool GUIApplication::notify (QObject * receiver, QEvent * event)
{
#if 0 // QProfile Record Test
    QProfileStarter* profile_ptr_auto = nullptr;
    if (QProfile::isCurOpFinished == 1)
    {
        auto const curThread = QThread::currentThread();
        if (auto customEvent = dynamic_cast<QPEvent*>(event))
        {
            QProfile::isCurOpFinished = 0;
        }
        else
        {
            if (!event)
            {
                return 0;
            }
            const char* cName = curThread->metaObject()->className();
            const char* receiverName = receiver->metaObject()->className();
            const char* evName = getEventName(event->type());

            QString qsR = QString::fromLocal8Bit(receiverName);
            QString qsE = QString::fromLocal8Bit(evName);
            qsR += "-";
            qsR += qsE;
            QByteArray qsRQB = qsR.toLocal8Bit();
            auto qsRQBLen = qsRQB.size() + 1;
            int alloqsRQBLen = qsRQBLen + 1;
            char* resqsRQBName = new char[alloqsRQBLen];
            const char* cnRQB = qsRQB.constData();
            memcpy(resqsRQBName, cnRQB, qsRQBLen);
            resqsRQBName[qsRQBLen] = 0;

            if (qsE.contains("WindowActivate") && qsR.contains("MainWindow"))
            {
                QProfile::isCurOpFinished = 0;
            }
#if 1
            //if (qsR.contains("MainWindow") && qsE.contains("UpdateRequest"))
            {
                static QProfileCache<int> qpCache;
                QProfile* var = qpCache.createQProFile(cName, 1, curThread); //receiver
                profile_ptr_auto = new QProfileStarter(var);
            }
#else	
            QProfile* var = new QProfile(resqsRQBName, true, __FILE__, __LINE__);
            profile_ptr_auto = new QProfileStarter(var);
#endif		
            //delete[] resName;
        }
    }
    auto scope = qScopeGuard([&]()
        {
            delete profile_ptr_auto;
        });

#endif// QProfile Record Test

    if (!receiver) {
        Base::Console().Log("GUIApplication::notify: Unexpected null receiver, event type: %d\n",
            (int)event->type());
        return false;
    }
    try {
        if (event->type() == Spaceball::ButtonEvent::ButtonEventType ||
            event->type() == Spaceball::MotionEvent::MotionEventType)
            return processSpaceballEvent(receiver, event);
        else
            return QApplication::notify(receiver, event);
    }
    catch (const Base::SystemExitException &e) {
        caughtException.reset(new Base::SystemExitException(e));
        qApp->exit(e.getExitCode());
        return true;
    }
    catch (const Base::Exception& e) {
        Base::Console().Error("Unhandled Base::Exception caught in GUIApplication::notify.\n"
                              "The error message is: %s\n", e.what());
    }
    catch (const std::exception& e) {
        Base::Console().Error("Unhandled std::exception caught in GUIApplication::notify.\n"
                              "The error message is: %s\n", e.what());
    }
    catch (...) {
        Base::Console().Error("Unhandled unknown exception caught in GUIApplication::notify.\n");
    }

    // Print some more information to the log file (if active) to ease bug fixing
    try {
        std::stringstream dump;
        dump << "The event type " << (int)event->type() << " was sent to "
             << receiver->metaObject()->className() << "\n";
        dump << "Object tree:\n";
        if (receiver->isWidgetType()) {
            QWidget* w = qobject_cast<QWidget*>(receiver);
            while (w) {
                dump << "\t";
                dump << w->metaObject()->className();
                QString name = w->objectName();
                if (!name.isEmpty())
                    dump << " (" << (const char*)name.toUtf8() << ")";
                w = w->parentWidget();
                if (w)
                    dump << " is child of\n";
            }
            std::string str = dump.str();
            Base::Console().Log("%s",str.c_str());
        }
    }
    catch (...) {
        Base::Console().Log("Invalid recipient and/or event in GUIApplication::notify\n");
    }

    return true;
}

void GUIApplication::commitData(QSessionManager &manager)
{
    if (manager.allowsInteraction()) {
        if (!Gui::getMainWindow()->close()) {
            // cancel the shutdown
            manager.release();
            manager.cancel();
        }
    }
    else {
        // no user interaction allowed, thus close all documents and
        // the main window
        App::GetApplication().closeAllDocuments();
        Gui::getMainWindow()->close();
    }
}

bool GUIApplication::event(QEvent * ev)
{
    if (ev->type() == QEvent::FileOpen) {
        QString file = static_cast<QFileOpenEvent*>(ev)->file();
        QFileInfo fi(file);
        if (fi.suffix().toLower() == QLatin1String("fcstd")) {
            QByteArray fn = file.toUtf8();
            Application::Instance->open(fn, "FreeCAD");
            return true;
        }
    }

    return GUIApplicationNativeEventAware::event(ev);
}

// ----------------------------------------------------------------------------

class GUISingleApplication::Private {
public:
    explicit Private(GUISingleApplication *q_ptr)
      : q_ptr(q_ptr)
      , timer(new QTimer(q_ptr))
      , server(nullptr)
      , running(false)
    {
        timer->setSingleShot(true);
        std::string exeName = App::Application::getExecutableName();
        serverName = QString::fromStdString(exeName);
    }

    ~Private()
    {
        if (server)
            server->close();
        delete server;
    }

    void setupConnection()
    {
        QLocalSocket socket;
        socket.connectToServer(serverName);
        if (socket.waitForConnected(1000)) {
            this->running = true;
        }
        else {
            startServer();
        }
    }

    void startServer()
    {
        // Start a QLocalServer to listen for connections
        server = new QLocalServer();
        QObject::connect(server, &QLocalServer::newConnection,
                         q_ptr, &GUISingleApplication::receiveConnection);
        // first attempt
        if (!server->listen(serverName)) {
            if (server->serverError() == QAbstractSocket::AddressInUseError) {
                // second attempt
                server->removeServer(serverName);
                server->listen(serverName);
            }
        }
        if (server->isListening()) {
            Base::Console().Log("Local server '%s' started\n", qPrintable(serverName));
        }
        else {
            Base::Console().Log("Local server '%s' failed to start\n", qPrintable(serverName));
        }
    }

    GUISingleApplication *q_ptr;
    QTimer *timer;
    QLocalServer *server;
    QString serverName;
    QList<QByteArray> messages;
    bool running;
};

GUISingleApplication::GUISingleApplication(int & argc, char ** argv)
    : GUIApplication(argc, argv),
      d_ptr(new Private(this))
{
    d_ptr->setupConnection();
    connect(d_ptr->timer, &QTimer::timeout, this, &GUISingleApplication::processMessages);
}

GUISingleApplication::~GUISingleApplication()
{
}

bool GUISingleApplication::isRunning() const
{
    return d_ptr->running;
}

bool GUISingleApplication::sendMessage(const QByteArray &message, int timeout)
{
    QLocalSocket socket;
    bool connected = false;
    for(int i = 0; i < 2; i++) {
        socket.connectToServer(d_ptr->serverName);
        connected = socket.waitForConnected(timeout/2);
        if (connected || i > 0)
            break;
        int ms = 250;
#if defined(Q_OS_WIN)
        Sleep(DWORD(ms));
#else
        usleep(ms*1000);
#endif
    }
    if (!connected)
        return false;

    QDataStream ds(&socket);
    ds << message;
    socket.waitForBytesWritten(timeout);
    return true;
}

void GUISingleApplication::receiveConnection()
{
    QLocalSocket *socket = d_ptr->server->nextPendingConnection();
    if (!socket)
        return;

    connect(socket, &QLocalSocket::disconnected,
            socket, &QLocalSocket::deleteLater);
    if (socket->waitForReadyRead()) {
        QDataStream in(socket);
        if (!in.atEnd()) {
            d_ptr->timer->stop();
            QByteArray message;
            in >> message;
            Base::Console().Log("Received message: %s\n", message.constData());
            d_ptr->messages.push_back(message);
            d_ptr->timer->start(1000);
        }
    }

    socket->disconnectFromServer();
}

void GUISingleApplication::processMessages()
{
    QList<QByteArray> msg = d_ptr->messages;
    d_ptr->messages.clear();
    Q_EMIT messageReceived(msg);
}

// ----------------------------------------------------------------------------

WheelEventFilter::WheelEventFilter(QObject* parent)
  : QObject(parent)
{
}

bool WheelEventFilter::eventFilter(QObject* obj, QEvent* ev)
{
    if (qobject_cast<QComboBox*>(obj) && ev->type() == QEvent::Wheel)
        return true;
    auto sb = qobject_cast<QAbstractSpinBox*>(obj);
    if (sb) {
        if (ev->type() == QEvent::Show) {
            sb->setFocusPolicy(Qt::StrongFocus);
        }
        else if (ev->type() == QEvent::Wheel) {
            return !sb->hasFocus();
        }
    }
    return false;
}

#include "moc_GuiApplication.cpp"
