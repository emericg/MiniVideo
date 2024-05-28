#include "hexfinddialog.h"
#include "../qhexview.h"
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QSpacerItem>
#include <QMessageBox>
#include <QLabel>
#include <QPair>
#include <QList>
#include <limits>

const QString HexFindDialog::BUTTONBOX  = "qhexview_buttonbox";
const QString HexFindDialog::CBFINDMODE = "qhexview_cbfindmode";
const QString HexFindDialog::LEFIND     = "qhexview_lefind";
const QString HexFindDialog::LEREPLACE  = "qhexview_lereplace";
const QString HexFindDialog::HLAYOUT    = "qhexview_hlayout";
const QString HexFindDialog::GBOPTIONS  = "qhexview_gboptions";
const QString HexFindDialog::RBALL      = "qhexview_rball";
const QString HexFindDialog::RBFORWARD  = "qhexview_rbforward";
const QString HexFindDialog::RBBACKWARD = "qhexview_rbbackward";

HexFindDialog::HexFindDialog(Type type, QHexView *parent) : QDialog{parent}, m_type{type}
{
    m_hexvalidator = new QRegularExpressionValidator(QRegularExpression{"[0-9A-Fa-f ]+"}, this);
    m_hexpvalidator = new QRegularExpressionValidator(QRegularExpression{"[0-9A-Fa-f \\?]+"}, this);
    m_dblvalidator = new QDoubleValidator(this);
    m_intvalidator = new QIntValidator(this);

    this->setWindowTitle(type == Type::Replace ? tr("Replace...") : tr("Find..."));

    auto* vlayout = new QVBoxLayout(this);
    auto* gridlayout = new QGridLayout();

    auto* cbfindmode = new QComboBox(this);
    cbfindmode->setObjectName(HexFindDialog::CBFINDMODE);
    cbfindmode->addItem("Text", static_cast<int>(QHexFindMode::Text));
    cbfindmode->addItem("Hex", static_cast<int>(QHexFindMode::Hex));
    cbfindmode->addItem("Int", static_cast<int>(QHexFindMode::Int));
    cbfindmode->addItem("Float", static_cast<int>(QHexFindMode::Float));

    QLineEdit* lereplace = nullptr, *lefind = new QLineEdit(this);
    lefind->setObjectName(HexFindDialog::LEFIND);

    gridlayout->addWidget(new QLabel(tr("Mode:"), this), 0, 0, Qt::AlignRight);
    gridlayout->addWidget(cbfindmode, 0, 1);
    gridlayout->addWidget(new QLabel(tr("Find:"), this), 1, 0, Qt::AlignRight);
    gridlayout->addWidget(lefind, 1, 1);

    if(type == Type::Replace)
    {
        lereplace = new QLineEdit(this);
        lereplace->setObjectName(HexFindDialog::LEREPLACE);

        gridlayout->addWidget(new QLabel(tr("Replace:"), this), 2, 0, Qt::AlignRight);
        gridlayout->addWidget(lereplace, 2, 1);
    }

    vlayout->addLayout(gridlayout);

    auto* gboptions = new QGroupBox(this);
    gboptions->setObjectName(HexFindDialog::GBOPTIONS);
    gboptions->setTitle(tr("Options"));
    gboptions->setLayout(new QVBoxLayout());

    QGroupBox* gbdirection = new QGroupBox(this);
    gbdirection->setTitle(tr("Find direction"));
    auto* gbvlayout = new QVBoxLayout(gbdirection);

    auto* rball = new QRadioButton("All", gbdirection);
    rball->setObjectName(HexFindDialog::RBALL);
    auto* rbforward = new QRadioButton("Forward", gbdirection);
    rbforward->setObjectName(HexFindDialog::RBFORWARD);
    rbforward->setChecked(true);
    auto* rbbackward = new QRadioButton("Backward", gbdirection);
    rbbackward->setObjectName(HexFindDialog::RBBACKWARD);

    gbvlayout->addWidget(rball);
    gbvlayout->addWidget(rbforward);
    gbvlayout->addWidget(rbbackward);
    gbvlayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    auto* hlayout = new QHBoxLayout();
    hlayout->setObjectName(HexFindDialog::HLAYOUT);
    hlayout->addWidget(gboptions, 1);
    hlayout->addWidget(gbdirection);
    vlayout->addLayout(hlayout, 1);

    auto* buttonbox = new QDialogButtonBox(this);
    buttonbox->setOrientation(Qt::Horizontal);

    if(type == Type::Replace) buttonbox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel);
    else buttonbox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    buttonbox->setObjectName(HexFindDialog::BUTTONBOX);
    buttonbox->button(QDialogButtonBox::Ok)->setEnabled(false);
    buttonbox->button(QDialogButtonBox::Ok)->setText(tr("Find"));

    if(type == Type::Replace)
    {
        buttonbox->button(QDialogButtonBox::Apply)->setEnabled(false);
        buttonbox->button(QDialogButtonBox::Apply)->setText(tr("Replace"));
    }

    vlayout->addWidget(buttonbox);

    connect(lefind, &QLineEdit::textChanged, this, &HexFindDialog::validateActions);
    connect(cbfindmode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HexFindDialog::updateFindOptions);
    connect(buttonbox, &QDialogButtonBox::accepted, this, &HexFindDialog::find);
    connect(buttonbox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(parent, &QHexView::positionChanged, this, [this]() { m_startoffset = -1; });

    if(lereplace)
    {
        connect(buttonbox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &HexFindDialog::replace);
        connect(lereplace, &QLineEdit::textChanged, this, &HexFindDialog::validateActions);
    }

    this->updateFindOptions(-1);
}

QHexView* HexFindDialog::hexView() const { return qobject_cast<QHexView*>(this->parentWidget()); }

void HexFindDialog::updateFindOptions(int)
{
    static const QList<QPair<QString, unsigned int>> INT_TYPES = {
        qMakePair<QString, unsigned int>("(any)", 0),
        qMakePair<QString, unsigned int>("8",  QHexFindOptions::Int8),
        qMakePair<QString, unsigned int>("16", QHexFindOptions::Int16),
        qMakePair<QString, unsigned int>("32", QHexFindOptions::Int32),
        qMakePair<QString, unsigned int>("64", QHexFindOptions::Int64)
    };

    static const QList<QPair<QString, unsigned int>> FLOAT_TYPES = {
        qMakePair<QString, unsigned int>("float",  QHexFindOptions::Float),
        qMakePair<QString, unsigned int>("double", QHexFindOptions::Double)
    };

    QGroupBox* gboptions = this->findChild<QGroupBox*>(HexFindDialog::GBOPTIONS);
    qDeleteAll(gboptions->findChildren<QWidget*>());

    QLineEdit* lefind = this->findChild<QLineEdit*>(HexFindDialog::LEFIND);
    QLineEdit* lereplace = this->findChild<QLineEdit*>(HexFindDialog::LEREPLACE);
    lefind->clear();
    if(lereplace) lereplace->clear();

    bool ok = false;
    QHexFindMode mode = static_cast<QHexFindMode>(this->findChild<QComboBox*>(HexFindDialog::CBFINDMODE)->currentData().toInt(&ok));
    if(!ok) return;

    QVBoxLayout* vlayout = qobject_cast<QVBoxLayout*>(gboptions->layout());

    m_findoptions = QHexFindOptions::None;
    m_oldidxbits = m_oldidxendian = -1;

    switch(mode)
    {
        case QHexFindMode::Text: {
            lefind->setValidator(nullptr);
            if(lereplace) lereplace->setValidator(nullptr);

            auto* cbcasesensitive = new QCheckBox("Case sensitive", gboptions);

            connect(cbcasesensitive, &QCheckBox::stateChanged, this, [this](int state) {
                if(state == Qt::Checked) m_findoptions |= QHexFindOptions::CaseSensitive;
                else m_findoptions &= ~QHexFindOptions::CaseSensitive;
            });

            vlayout->addWidget(cbcasesensitive);
            gboptions->setVisible(true);
            break;
        }

        case QHexFindMode::Hex: {
            lefind->setValidator(m_hexpvalidator);
            if(lereplace) lereplace->setValidator(m_hexvalidator);
            gboptions->setVisible(false);
            break;
        }

        case QHexFindMode::Int: {
            lefind->setValidator(m_intvalidator);
            if(lereplace) lereplace->setValidator(m_intvalidator);

            auto* cbbits = new QComboBox();
            for(const auto& it : INT_TYPES) cbbits->addItem(it.first, it.second);

            connect(cbbits, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, cbbits](int index) {
                if(m_oldidxbits > -1) m_findoptions &= ~cbbits->itemData(m_oldidxbits).toUInt();
                m_findoptions |= cbbits->itemData(index).toUInt();
                m_oldidxbits = index;
            });

            auto* cbendian = new QComboBox();
            cbendian->addItem("Little Endian", 0);
            cbendian->addItem("Big Endian", QHexFindOptions::BigEndian);

            connect(cbendian, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, cbendian](int index) {
                if(m_oldidxendian > -1) m_findoptions &= ~cbendian->itemData(m_oldidxendian).toUInt();
                m_findoptions |= cbendian->itemData(index).toUInt();
                m_oldidxendian = index;
            });

            QGridLayout* gl = new QGridLayout();
            gl->addWidget(new QLabel("Type:"), 0, 0, Qt::AlignRight);
            gl->addWidget(cbbits, 0, 1);
            gl->addWidget(new QLabel("Endian:"), 1, 0, Qt::AlignRight);
            gl->addWidget(cbendian, 1, 1);
            vlayout->addLayout(gl);
            gboptions->setVisible(true);
            break;
        }

        case QHexFindMode::Float: {
            lefind->setValidator(m_dblvalidator);
            if(lereplace) lereplace->setValidator(m_dblvalidator);

            bool first = true;
            QVBoxLayout* vl = new QVBoxLayout();

            for(const auto& ft : FLOAT_TYPES) {
                auto* rb = new QRadioButton(ft.first);
                rb->setChecked(first);
                vl->addWidget(rb);
                first = false;
            }

            vlayout->addLayout(vl);
            gboptions->setVisible(true);
            break;
        }
    }
}

bool HexFindDialog::validateIntRange(uint v) const
{
    if(m_findoptions & QHexFindOptions::Int8) return !(v > std::numeric_limits<quint8>::max());
    if(m_findoptions & QHexFindOptions::Int16) return !(v > std::numeric_limits<quint16>::max());
    if(m_findoptions & QHexFindOptions::Int32) return !(v > std::numeric_limits<quint32>::max());
    return true;
}

void HexFindDialog::checkResult(const QString& q, qint64 offset, QHexFindDirection fd)
{
    if(offset == -1)
    {
        QMessageBox::information(this, tr("Not found"), tr("Cannot find '%1'").arg(q));
        return;
    }

    if(fd == QHexFindDirection::Backward) m_startoffset = this->hexView()->selectionStartOffset() - 1;
    else m_startoffset = this->hexView()->selectionEndOffset() + 1;
}

void HexFindDialog::validateActions()
{
    auto mode = static_cast<QHexFindMode>(this->findChild<QComboBox*>(HexFindDialog::CBFINDMODE)->currentData().toUInt());
    auto* lefind = this->findChild<QLineEdit*>(HexFindDialog::LEFIND);
    auto* lereplace = this->findChild<QLineEdit*>(HexFindDialog::LEREPLACE);
    auto* buttonbox = this->findChild<QDialogButtonBox*>(HexFindDialog::BUTTONBOX);

    bool findenable = false, replaceenable = false;

    switch(mode)
    {
        case QHexFindMode::Hex:
            findenable = QHexUtils::checkPattern(lefind->text());
            replaceenable = findenable;
            break;

        case QHexFindMode::Float: {
            lefind->text().toFloat(&findenable);
            if(lereplace && findenable) lereplace->text().toFloat(&replaceenable);
            break;
        }

        case QHexFindMode::Int: {
            auto v = lefind->text().toUInt(&findenable);
            if(findenable && !this->validateIntRange(v)) findenable = false;
            if(lereplace && findenable) lereplace->text().toUInt(&replaceenable);
            break;
        }

        default:
            findenable = !lefind->text().isEmpty();
            replaceenable = findenable;
            break;
    }

    if(lereplace) buttonbox->button(QDialogButtonBox::Apply)->setEnabled(replaceenable);
    buttonbox->button(QDialogButtonBox::Ok)->setEnabled(findenable);
}

void HexFindDialog::replace()
{
    QString q1;
    QHexFindMode mode;
    QHexFindDirection fd;

    if(!this->prepareOptions(q1, mode, fd)) return;

    QString q2 = this->findChild<QLineEdit*>(HexFindDialog::LEREPLACE)->text();
    auto offset = this->hexView()->hexCursor()->replace(q1, q2, m_startoffset > -1 ? m_startoffset : this->hexView()->offset(), mode, m_findoptions, fd);
    this->checkResult(q1, offset, fd);
}

void HexFindDialog::find()
{
    QString q;
    QHexFindMode mode;
    QHexFindDirection fd;

    if(!this->prepareOptions(q, mode, fd)) return;

    auto offset = this->hexView()->hexCursor()->find(q, m_startoffset > -1 ? m_startoffset : this->hexView()->offset(), mode, m_findoptions, fd);
    this->checkResult(q, offset, fd);
}

bool HexFindDialog::prepareOptions(QString& q, QHexFindMode& mode, QHexFindDirection& fd)
{
    q = this->findChild<QLineEdit*>(HexFindDialog::LEFIND)->text();
    mode = static_cast<QHexFindMode>(this->findChild<QComboBox*>(HexFindDialog::CBFINDMODE)->currentData().toUInt());

    if(mode == QHexFindMode::Hex && !QHexUtils::checkPattern(q))
    {
        QMessageBox::warning(this, tr("Pattern Error"), tr("Hex pattern '%1' is not valid").arg(q));
        return false;
    }

    if(this->findChild<QRadioButton*>(HexFindDialog::RBBACKWARD)->isChecked()) fd = QHexFindDirection::Backward;
    else if(this->findChild<QRadioButton*>(HexFindDialog::RBFORWARD)->isChecked()) fd = QHexFindDirection::Forward;
    else fd = QHexFindDirection::All;
    return true;
}
