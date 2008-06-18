#include "ibus-input-context.h"
#include "ibus-client.h"
#include <QtDebug>
#include <QInputMethodEvent>
#include <QTextCharFormat>

typedef QInputMethodEvent::Attribute QAttribute;

IBusInputContext::IBusInputContext (QObject *parent, IBusClient *client, QString &ic)
	: QInputContext (parent), client (client), ic (ic)
{
}

IBusInputContext::~IBusInputContext ()
{
	client->releaseInputContext (this);
}

bool
IBusInputContext::filterEvent (const QEvent *event)
{
#ifndef Q_WS_X11
	if (client->filterEvent (this, event))
		return true;
	return QInputContext::filterEvent (event);
#else
	return QInputContext::filterEvent (event);
#endif
}

QFont
IBusInputContext::font () const
{
	return QInputContext::font ();
}

QString
IBusInputContext::identifierName ()
{
	return QString ("ibus");
}

QString
IBusInputContext::language()
{
	return QString ("");
}

void
IBusInputContext::mouseHandler (int x, QMouseEvent *event)
{
	client->mouseHandler (this, x, event);
	QInputContext::mouseHandler (x, event);
}

void
IBusInputContext::reset()
{
	client->reset (this);
}

void
IBusInputContext::update ()
{
	QWidget *widget;
	QVariant value;
	QRect rect;
	QPoint topleft;

	if ((widget = focusWidget ()) == NULL)
		return;
	qDebug () << "update ()";
	value = widget->inputMethodQuery(Qt::ImMicroFocus);
	qDebug () << value;
	rect = value.toRect ();

	value = widget->inputMethodQuery(Qt::ImCursorPosition);
	qDebug () << value;
	rect.moveTo (value.toPoint ());

	topleft = widget->mapToGlobal(QPoint(0,0));
	rect.translate (topleft);

	client->setCursorLocation (this, rect);
	qDebug () << rect;

	// value = widget->inputMethodQuery(Qt::ImFont);
	// qDebug () << value;
	// value = widget->inputMethodQuery(Qt::ImCursorPosition);
	// qDebug () << value;
	// value = widget->inputMethodQuery(Qt::ImSurroundingText);
	// qDebug () << value;
	// value = widget->inputMethodQuery(Qt::ImCurrentSelection);
	// qDebug () << value;
}

bool
IBusInputContext::isComposing() const
{
	return client->isComposing (this);
}

void
IBusInputContext::setFocusWidget (QWidget *widget)
{
	QInputContext::setFocusWidget (widget);
	update ();
}

void
IBusInputContext::widgetDestroyed (QWidget *widget)
{
	QInputContext::widgetDestroyed (widget);
	update ();
}

#ifdef Q_WS_X11
bool
IBusInputContext::x11FilterEvent (QWidget *keywidget, XEvent *xevent)
{
	if (client->x11FilterEvent (this, keywidget, xevent))
		return true;
	return QInputContext::x11FilterEvent (keywidget, xevent);
}
#endif

void
IBusInputContext::setIC (QString ic)
{
	this->ic = ic;
}

QString
IBusInputContext::getIC ()
{
	return ic;
}

void
IBusInputContext::commitString (QString text)
{
	QInputMethodEvent event;
	event.setCommitString (text);
	sendEvent (event);
}

void
IBusInputContext::updatePreedit (QString text, QList <QList <quint32> > attr_list, int cursor_pos, bool show)
{
	qDebug () << text << cursor_pos << show;
	QList <QAttribute> qattrs;

	if (show) {
		// append cursor pos
		qattrs.append (QAttribute (QInputMethodEvent::Cursor, cursor_pos, true, 0));

		// append attributes
		for (QList <QList <quint32> >::iterator it = attr_list.begin (); it != attr_list.end(); ++ it) {

			QList <quint32> attr = *it;
			QTextCharFormat format;

			switch (attr[0]) {
			case 1: // underline
				format.setUnderlineStyle (QTextCharFormat::SingleUnderline);
				break;
			case 2: // foreground
				format.setForeground (QBrush (QColor (attr[1])));
				break;
			case 3: // background
				format.setBackground (QBrush (QColor (attr[1])));
				break;
			default:
				break;
			}

			QVariant value;
			value.setValue <QTextFormat> (format);
			qattrs.append (QAttribute (QInputMethodEvent::TextFormat, attr[2], attr[3] - attr[2], value));
		}
	}
	else {
		qattrs.append (QAttribute (QInputMethodEvent::Cursor, 0, true, 0));
		text = "";
	}

	QInputMethodEvent event (text, qattrs);
	sendEvent (event);
}
