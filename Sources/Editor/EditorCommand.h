#pragma once

#include <functional>
#include <queue>

class EditorCommand {
public:
	std::function<void()> Do;
	std::function<void()> Undo;
	std::string Info;
};

class EditorCommandStack {
	std::function<void()> _OnCmd;
	std::vector<EditorCommand> _Cmds;
	int _CmdIndex{0};

public:
	int GroupDepth{0};

	bool IsLastCmd() {
		return _CmdIndex == _Cmds.size() - 1;
	}

	void SetOnCmd(std::function<void()> callback) {
		_OnCmd = callback;
	}

	void RunCmd(EditorCommand cmd){
		_Cmds.resize(_CmdIndex+1);//resizes and also clear invalidated cmds
		_Cmds[_CmdIndex] = cmd;
		cmd.Do();
		_CmdIndex++;
		_OnCmd();
	}
	void Redo(){
		if (_CmdIndex >= _Cmds.size())return;
		auto& cmd = _Cmds[_CmdIndex];
		_CmdIndex++;
		cmd.Do();
		_OnCmd();
	}
	void Undo(){
		if (_CmdIndex <= 0)return;
		_CmdIndex--;
		auto& cmd = _Cmds[_CmdIndex];
		cmd.Undo();
		_OnCmd();
	}
};