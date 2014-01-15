#include "channels.h"
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QDoubleSpinBox>

Channels::Channels(QWidget * parent, ModelData & model):
  ModelPanel(parent, model)
{
  QGridLayout * gridLayout = new QGridLayout(this);

  int col = 1;
  if (GetEepromInterface()->getCapability(ChannelsName))
    addLabel(gridLayout, tr("Name"), col++);
  addLabel(gridLayout, tr("Offset"), col++);
  addLabel(gridLayout, tr("Min"), col++);
  addLabel(gridLayout, tr("Max"), col++);
  addLabel(gridLayout, tr("Invert"), col++);
  if (GetEepromInterface()->getCapability(PPMCenter))
    addLabel(gridLayout, tr("Center"), col++);
  if (GetEepromInterface()->getCapability(SYMLimits))
    addLabel(gridLayout, tr("Sym"), col++);

  for (int i=0; i<GetEepromInterface()->getCapability(Outputs); i++) {
    col = 0;

    // Channel label
    QLabel *label = new QLabel(this);
    label->setText(tr("Channel %1").arg(i+1));
    gridLayout->addWidget(label, i+1, col++, 1, 1);

    // Channel name
    int nameLen = GetEepromInterface()->getCapability(ChannelsName);
    if (nameLen > 0) {
      QLineEdit * name = new QLineEdit(this);
      name->setProperty("index", i);
      name->setMaxLength(nameLen);
      QRegExp rx(CHAR_FOR_NAMES_REGEX);
      name->setValidator(new QRegExpValidator(rx, this));
      name->setText(model.limitData[i].name);
      connect(name, SIGNAL(editingFinished()), this, SLOT(nameEdited()));
      gridLayout->addWidget(name, i+1, col++, 1, 1);
    }

    // Channel offset
    QDoubleSpinBox * offset = new QDoubleSpinBox(this);
    offset->setProperty("index", i);
    offset->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    offset->setAccelerated(true);
    offset->setDecimals(1);
    offset->setMinimum(-100);
    offset->setSingleStep(0.1);
    offset->setValue(float(model.limitData[i].offset) / 10);
    connect(offset, SIGNAL(editingFinished()), this, SLOT(offsetEdited()));
    gridLayout->addWidget(offset, i+1, col++, 1, 1);

    // Channel min
    QSpinBox * minSB = new QSpinBox(this);
    minSB->setProperty("index", i);
    minSB->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    minSB->setAccelerated(true);
    minSB->setMinimum(model.extendedLimits ? -125 : -100);
    minSB->setMaximum(0);
    minSB->setValue(model.limitData[i].min);
    connect(minSB, SIGNAL(editingFinished()), this, SLOT(minEdited()));
    gridLayout->addWidget(minSB, i+1, col++, 1, 1);

    // Channel max
    QSpinBox * maxSB = new QSpinBox(this);
    maxSB->setProperty("index", i);
    maxSB->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    maxSB->setAccelerated(true);
    maxSB->setMinimum(0);
    maxSB->setMaximum(model.extendedLimits ? 125 : 100);
    maxSB->setValue(model.limitData[i].max);
    connect(maxSB, SIGNAL(editingFinished()), this, SLOT(maxEdited()));
    gridLayout->addWidget(maxSB, i+1, col++, 1, 1);

    // Channel inversion
    QComboBox * invCB = new QComboBox(this);
    invCB->insertItems(0, QStringList() << tr("---") << tr("INV"));
    invCB->setProperty("index", i);
    invCB->setCurrentIndex((model.limitData[i].revert) ? 1 : 0);
    connect(invCB, SIGNAL(currentIndexChanged(int)), this, SLOT(invEdited()));
    gridLayout->addWidget(invCB, i+1, col++, 1, 1);

    // PPM center
    if (GetEepromInterface()->getCapability(PPMCenter)) {
      QSpinBox * center = new QSpinBox(this);
      center->setProperty("index", i);
      center->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
      center->setMinimum(1375);
      center->setMaximum(1625);
      center->setValue(1500);
      center->setValue(model.limitData[i].ppmCenter + 1500);
      connect(center, SIGNAL(editingFinished()), this, SLOT(ppmcenterEdited()));
      gridLayout->addWidget(center, i+1, col++, 1, 1);
    }

    // Symetrical limits
    if (GetEepromInterface()->getCapability(SYMLimits)) {
      QCheckBox * symlimits = new QCheckBox(this);
      symlimits->setProperty("index", i);
      symlimits->setChecked(model.limitData[i].symetrical);
      connect(symlimits, SIGNAL(toggled(bool)), this, SLOT(symlimitsEdited()));
      gridLayout->addWidget(symlimits, i+1, col++, 1, 1);
    }
  }

  // setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  // setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

Channels::~Channels()
{
}

void Channels::symlimitsEdited()
{
  QCheckBox * ckb = qobject_cast<QCheckBox*>(sender());
  int index = ckb->property("index").toInt();
  model.limitData[index].symetrical = (ckb->checkState() ? 1 : 0);
  emit modified();
}

void Channels::nameEdited()
{
  if (!lock) {
    lock = true;
    QLineEdit *le = qobject_cast<QLineEdit*>(sender());
    int index = le->property("index").toInt();
    strcpy(model.limitData[index].name, le->text().toAscii());
    lock = false;
    emit modified();
  }
}

void Channels::offsetEdited()
{
  QDoubleSpinBox *dsb = qobject_cast<QDoubleSpinBox*>(sender());
  int index = dsb->property("index").toInt();
  model.limitData[index].offset = round(dsb->value()*10);
  emit modified();
}

void Channels::minEdited()
{
  QSpinBox *sb = qobject_cast<QSpinBox*>(sender());
  int index = sb->property("index").toInt();
  model.limitData[index].min = sb->value();
  emit modified();
}

void Channels::maxEdited()
{
  QSpinBox *sb = qobject_cast<QSpinBox*>(sender());
  int index = sb->property("index").toInt();
  model.limitData[index].max = sb->value();
  emit modified();
}

void Channels::invEdited()
{
  QComboBox *cb = qobject_cast<QComboBox*>(sender());
  int index = cb->property("index").toInt();
  model.limitData[index].revert = cb->currentIndex();
  emit modified();
}

void Channels::ppmcenterEdited()
{
  QSpinBox *sb = qobject_cast<QSpinBox*>(sender());
  int index = sb->property("index").toInt();
  model.limitData[index].ppmCenter = sb->value() - 1500;
  emit modified();
}

