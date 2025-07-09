/****************************************************************************
** Meta object code from reading C++ file 'waveformview.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../waveformview.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'waveformview.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN12WaveformViewE_t {};
} // unnamed namespace

template <> constexpr inline auto WaveformView::qt_create_metaobjectdata<qt_meta_tag_ZN12WaveformViewE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "WaveformView",
        "markerPositionChanged",
        "",
        "seconds",
        "setSamples",
        "QList<double>",
        "samples",
        "sampleRate",
        "setMarkerPosition"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'markerPositionChanged'
        QtMocHelpers::SignalData<void(double)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 3 },
        }}),
        // Slot 'setSamples'
        QtMocHelpers::SlotData<void(const QVector<double> &, quint32)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 5, 6 }, { QMetaType::UInt, 7 },
        }}),
        // Slot 'setMarkerPosition'
        QtMocHelpers::SlotData<void(double)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 3 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<WaveformView, qt_meta_tag_ZN12WaveformViewE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject WaveformView::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12WaveformViewE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12WaveformViewE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN12WaveformViewE_t>.metaTypes,
    nullptr
} };

void WaveformView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<WaveformView *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->markerPositionChanged((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 1: _t->setSamples((*reinterpret_cast< std::add_pointer_t<QList<double>>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<quint32>>(_a[2]))); break;
        case 2: _t->setMarkerPosition((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<double> >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (WaveformView::*)(double )>(_a, &WaveformView::markerPositionChanged, 0))
            return;
    }
}

const QMetaObject *WaveformView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WaveformView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12WaveformViewE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int WaveformView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void WaveformView::markerPositionChanged(double _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}
QT_WARNING_POP
