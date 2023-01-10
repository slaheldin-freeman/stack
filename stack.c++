#include <stack>
#include <iostream>
#include <memory>

// ----- the Command Interface -----
class ICommand {
public:
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual void redo() = 0;
};

// ----- the MODEL -----
class Tv {
    bool mOn;
    int mChannel;

public:
    Tv() {}
    void switchOn() { mOn = true; }
    void switchOff() { mOn = false; }

    void switchChannel(int channel) {
        mChannel = channel;
    }

    bool isOn() { return mOn; }
    int getChannel() { return mChannel; }
};

// ----- concrete ICommand commands -----
class TvOnCommand : public ICommand {
    Tv* mTv;

public:
    TvOnCommand(Tv& tv) :mTv(&tv) {}
    void execute() { mTv->switchOn(); }
    void undo() { mTv->switchOff(); }
    void redo() { mTv->switchOn(); }
};

class TvOffCommand : public ICommand {
    TvOnCommand mTvOnCommand;              // reuse complementary command
public:
    TvOffCommand(Tv& tv) :mTvOnCommand(tv) {}
    void execute() { mTvOnCommand.undo(); }
    void undo() { mTvOnCommand.execute(); }
    void redo() { mTvOnCommand.undo(); }
};

class TvSwitchChannelCommand : public ICommand {
    Tv* mTv;
    int mOldChannel;
    int mNewChannel;

public:
    TvSwitchChannelCommand(Tv* tv, int channel) {
        mTv = tv;
        mNewChannel = channel;
    }

    void execute() {
        mOldChannel = mTv->getChannel();
        mTv->switchChannel(mNewChannel);
    }

    void undo() {
        mTv->switchChannel(mOldChannel);
    }

    void redo() {
        mTv->switchChannel(mNewChannel);
    }
};

// ----- our CONTROLLER with undo/redo -----
typedef std::stack<std::shared_ptr<ICommand> > commandStack_t;

class CommandManager {
    commandStack_t mUndoStack;
    commandStack_t mRedoStack;

public:
    CommandManager() {}

    void executeCmd(std::shared_ptr<ICommand> command) {
        mRedoStack = commandStack_t(); // clear the redo stack
        command->execute();
        mUndoStack.push(command);
    }

    void undo() {
        if (mUndoStack.size() <= 0) {
            return;
        }
        mUndoStack.top()->undo();          // undo most recently executed command
        mRedoStack.push(mUndoStack.top()); // add undone command to undo stack
        mUndoStack.pop();                  // remove top entry from undo stack
    }

    void redo() {
        if (mRedoStack.size() <= 0) {
            return;
        }
        mRedoStack.top()->redo();          // redo most recently executed command
        mUndoStack.push(mRedoStack.top()); // add undone command to redo stack
        mRedoStack.pop();                  // remove top entry from redo stack
    }
};

int main() {
    using namespace std;

    Tv tv;
    CommandManager commandManager;

    std::shared_ptr<ICommand> c1(new TvSwitchChannelCommand(&tv, 1)); // create command for switching to channel 1
    commandManager.executeCmd(c1);
    std::cout << "switched to channel " << tv.getChannel() << std::endl;

    std::shared_ptr<ICommand> c2(new TvSwitchChannelCommand(&tv, 2)); // create command for switching to channel 2
    commandManager.executeCmd(c2);
    std::cout << "switched to channel: " << tv.getChannel() << std::endl;

    std::shared_ptr<ICommand> c3(new TvSwitchChannelCommand(&tv, 3)); // create command for switching to channel 3
    commandManager.executeCmd(c3);
    std::cout << "switched to channel: " << tv.getChannel() << std::endl;

    std::cout << "undoing..." << std::endl;
    commandManager.undo();
    std::cout << "current channel: " << tv.getChannel() << std::endl;

    std::cout << "undoing..." << std::endl;
    commandManager.undo();
    std::cout << "current channel: " << tv.getChannel() << std::endl;

    std::cout << "redoing..." << std::endl;
    commandManager.redo();
    std::cout << "current channel: " << tv.getChannel() << std::endl;

    std::cout << "redoing..." << std::endl;
    commandManager.redo();
    std::cout << "current channel: " << tv.getChannel() << std::endl;

    return 0;
}
If you compileand run the code above, the out put is :

switched to channel 1
switched to channel : 2
switched to channel : 3
undoing...
current channel : 2
undoing...
current channel : 1
redoing...
current channel : 2
redoing...
current channel : 3
