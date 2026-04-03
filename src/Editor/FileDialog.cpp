#include "Editor.hpp"

FileDialog::FileDialog()
    : mState(InitGuiWindowFileDialog(NULL)),
      mMode(kNone)
{
}

void FileDialog::Activate(FileDialogMode mode)
{
    mState.saveFileMode = mode & kSaveMode;
    mState.windowActive = true;
    mState.windowBounds.x = 0;
    mState.windowBounds.y = 0;
    mMode = mode;
}

void FileDialog::Deactivate()
{
    mState.SelectFilePressed = false;
    mMode = kNone;
}

void FileDialog::Draw()
{
    GuiWindowFileDialog(&mState);
}

bool FileDialog::IsFileSelected()
{
    return mState.SelectFilePressed;
}

bool FileDialog::IsActive()
{
    return mMode != kNone;
}

const char * FileDialog::GetSelectedFileName()
{
    return TextFormat("%s/%s", mState.dirPathText, mState.fileNameText);
}

FileDialog::FileDialogMode FileDialog::GetMode()
{
    return mMode;
}
