#pragma once

#include <QUndoCommand>

#include <QComboBox>
#include <QUndoCommand>

// == ComboBoxUndoCommand =====================================================
// ============================================================================

class ComboBoxUndoCommand : public QUndoCommand {
public:
    ComboBoxUndoCommand(QComboBox* comboBox, int old_index, int new_index,
        std::function<void(int index)> callback, QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    QComboBox* combo_;
    int old_index_;
    int new_index_;
    std::function<void(int)> callback_;
};

class VariantUndoCommand : public QUndoCommand {
public:
    VariantUndoCommand(const QVariant& last, const QVariant& next, std::function<void(const QVariant& value)> setter);

    void undo() override;
    void redo() override;

private:
    QVariant last_;
    QVariant next_;
    std::function<void(const QVariant& value)> setter_;
};
