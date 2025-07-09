/****************************************************************************
** Meta object code from reading C++ file 'audiomodel.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../audiomodel.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'audiomodel.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN10AudioModelE_t {};
} // unnamed namespace

template <> constexpr inline auto AudioModel::qt_create_metaobjectdata<qt_meta_tag_ZN10AudioModelE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "AudioModel",
        "metadataReady",
        "",
        "AudioModel::Meta",
        "m",
        "waveformReady",
        "QList<double>",
        "samples",
        "sampleRate",
        "spectrumReady",
        "freq",
        "amp",
        "errorOccurred",
        "error"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'metadataReady'
        QtMocHelpers::SignalData<void(const AudioModel::Meta &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'waveformReady'
        QtMocHelpers::SignalData<void(const QVector<double> &, quint32)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 6, 7 }, { QMetaType::UInt, 8 },
        }}),
        // Signal 'spectrumReady'
        QtMocHelpers::SignalData<void(const QVector<double> &, const QVector<double> &)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 6, 10 }, { 0x80000000 | 6, 11 },
        }}),
        // Signal 'errorOccurred'
        QtMocHelpers::SignalData<void(const QString &)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 13 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<AudioModel, qt_meta_tag_ZN10AudioModelE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject AudioModel::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10AudioModelE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10AudioModelE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10AudioModelE_t>.metaTypes,
    nullptr
} };

void AudioModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AudioModel *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->metadataReady((*reinterpret_cast< std::add_pointer_t<AudioModel::Meta>>(_a[1]))); break;
        case 1: _t->waveformReady((*reinterpret_cast< std::add_pointer_t<QList<double>>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<quint32>>(_a[2]))); break;
        case 2: _t->spectrumReady((*reinterpret_cast< std::add_pointer_t<QList<double>>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QList<double>>>(_a[2]))); break;
        case 3: _t->errorOccurred((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
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
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 1:
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<double> >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (AudioModel::*)(const AudioModel::Meta & )>(_a, &AudioModel::metadataReady, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (AudioModel::*)(const QVector<double> & , quint32 )>(_a, &AudioModel::waveformReady, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (AudioModel::*)(const QVector<double> & , const QVector<double> & )>(_a, &AudioModel::spectrumReady, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (AudioModel::*)(const QString & )>(_a, &AudioModel::errorOccurred, 3))
            return;
    }
}

const QMetaObject *AudioModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AudioModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10AudioModelE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int AudioModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void AudioModel::metadataReady(const AudioModel::Meta & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void AudioModel::waveformReady(const QVector<double> & _t1, quint32 _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2);
}

// SIGNAL 2
void AudioModel::spectrumReady(const QVector<double> & _t1, const QVector<double> & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}

// SIGNAL 3
void AudioModel::errorOccurred(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}
QT_WARNING_POP
