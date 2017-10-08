Come compilare il progetto.
1)Proprietà->C/C++->Preprocessore->definizioni processore
	WIN32
	_DEBUG
	_WINDOWS
	_CRT_SECURE_NO_WARNINGS
	WIN32_LEAN_AND_MEAN
	_WIN32_WINNT=0x0501
2)Scaricare tramite nuget il pacchetto boost-asio 1.64.0
3)Proprietà->Directory di VC++->Directory di inclusione e aggiungere "wxWidgets-3.1\include\msvc"
4)Proprietà->Directory di VC++->Directory di inclusione e aggiungere "wxWidgets-3.1\include\"
5)Proprietà->Directory di VC++->Directory librerie e aggiungere "wxWidgets-3.1\lib\vc_lib"
5)Proprietà->Linker->Directory Librerie Aggiuntive e aggiungere boost_1_64_0\stage\lib