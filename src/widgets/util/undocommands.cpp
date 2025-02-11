#include "undocommands.h"

// == ComboBoxUndoCommand =====================================================
// ============================================================================

ComboBoxUndoCommand::ComboBoxUndoCommand(QComboBox* comboBox, int old_index, int new_index,
    std::function<void(int)> callback, QUndoCommand* parent)
    : QUndoCommand(parent)
    , combo_{comboBox}
    , old_index_{old_index}
    , new_index_{new_index}
    , callback_{callback}
{
    setText(QString("Change selection to '%1'")
            .arg(combo_->itemText(new_index_)));
}

void ComboBoxUndoCommand::undo()
{
    combo_->blockSignals(true);
    combo_->setCurrentIndex(old_index_);
    combo_->setProperty("previous_index", old_index_);
    callback_(old_index_);
    combo_->blockSignals(false);
}

void ComboBoxUndoCommand::redo()
{
    combo_->blockSignals(true);
    combo_->setCurrentIndex(new_index_);
    combo_->setProperty("previous_index", new_index_);
    callback_(new_index_);
    combo_->blockSignals(false);
}

VariantUndoCommand::VariantUndoCommand(const QVariant& last, const QVariant& next, std::function<void(const QVariant&)> setter)
    : last_{last}
    , next_{next}
    , setter_{std::move(setter)}
{
}

void VariantUndoCommand::undo()
{
    setter_(last_);
}

void VariantUndoCommand::redo()
{
    setter_(next_);
}
