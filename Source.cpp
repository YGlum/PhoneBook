/*
Это моё исполнение тестового задания.
И если вам дорого ваше здоровье, аккуратно.

Итак основной функционал есть добавить, удалить, пойск, сохранение и загрузка (из файла), для GUI использовался SDL2 (за временным неймением других средств)
Среда выполнения (C4droid 5.96);
Весь код здесь не хотелось постояно прыгать между файлами
Откоментировал если кому интересно

Работоспособность будет показана в небольшой видюшке

*/


#include <string>
#include <fstream>

#include <map>
#include <vector>


#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

using namespace std;

//Фаил лога для отладки
ofstream LogStream("log.txt");

//Первичный размер экрана
SDL_Rect Screen{ 0, 0, 300, 300 };

//Структуры окна и поверхности рендераБ и шрифтов серавно они общие, уже(изначально юзал два а потом подумал х**ня какая-то)
SDL_Window *win = nullptr;
SDL_Renderer *renderer = nullptr;
TTF_Font *font = nullptr;

//Функций работы с GUIЖ
//Инициализаций SDL, включение необходимых элементов, загрузки текстур(для фона), ределинга текста, загрузки в рендер, отключения компонент
bool Init_SDL();
bool Init_ComponentSDL();

SDL_Texture *LoadImage(string file);
SDL_Texture *CreateText(string text);

void ApplySurface(int x, int y, SDL_Texture * tex);
void CloseP();


//Класс книги, кривой немного дизайн(GUI) и функционал тут смешан по двум причинам, неудобно в телефоне когда за 500 строк, случайно удалил класс с дизайном(востанавливать было лень) 
class ContactBook final
{
private:
	//Контейнер под записи <Имя, Номер> и итератор для работы
	map < string, string > Blotter_Map;
	map < string, string >::iterator iter = Blotter_Map.begin();

	//Для работы скорлинга(вроде так)
	int count = 0;

	//Поверхности текстур для Имени и Номера
	SDL_Texture *textNameT = nullptr;
	SDL_Texture *textNumberT = nullptr;

	//Размер одной записи (прямоугольная форма с кординатами верхнего левого и нижнего правого) 
	//Хотелось бы расчёт  "Screen.h - Screen.h / 100 * 80" вынести в отдельную функций дальше станет ясно почему, но не хотел заморачиваться
	SDL_Rect RecordingSize{ 0, Screen.h - Screen.h / 100 * 80, Screen.w, Screen.h / 100 * 10 };

public:
	//Методы добавления, удаления, поиска, чтения. записи
	void Add(string & Name, string & Number);
	void Delit(string & Name);
	string Find(string & Name);

	void ReadContactFile();
	void SaveContactFile();

	//А это для рисования, и проверки кординат касания
	void Draw();
	bool Touch(int x, int y);

	//Тут нужны для регулировки скорлинга
	void SetCount(int i)
	{
		count = i;
	};
	int GetCount()
	{
		return count;
	};
	int SizeMap()
	{
		return Blotter_Map.size();
	};
};

//Класс кнопки, прямоугольника с надписьюБ здесь кстати два в одном экономил место
class Button final
{
private:
	SDL_Texture *buttonText = nullptr;
	SDL_Rect ButtonSize;
	int typeButton;
public:
	//Конструктор принимает отределеный текст, свой кординаты, и тип (кнопка(0)/поле ввода(1))
	Button(SDL_Texture * bText, SDL_Rect * size, int type);

	//Методы рисования, замены текста и проверки кординат касания
	void Draw();
	void SetText(SDL_Texture * NewText);
	bool Touch(int x, int y);
};


//Точка входа
int main(int argv, char **argc)
{
	//Инициализация
	if (!Init_SDL())
		return 1;

	//Получение актуальных размеров моб. экрана
	SDL_DisplayMode Mode;
	SDL_GetCurrentDisplayMode(0, &Mode);
	{
		Screen.w = Mode.w;
		Screen.h = Mode.h;
	}

	//Компоненты
	if (!Init_ComponentSDL())
		return 1;

	int bW, bH;
	const int SW = Screen.w;
	const int SH = Screen.h;

	//Установка шрифта
	font = TTF_OpenFont("atari.ttf", 35);
	if (font == nullptr)
	{
		LogStream << "Ошибка шрифтов:\t" << TTF_GetError() << endl;
	}

	//Строки с изменением содержимого
	string inputName = "";
	string inputNumber = "";
	string inputFind = "";
	string resultFind = "";
	SDL_Texture *inputNameT = nullptr;
	SDL_Texture *inputNumberT = nullptr;
	SDL_Texture *inputFindT = nullptr;
	SDL_Texture *resultFindT = nullptr;

	//Постаяные записи
	const int n=7;
	SDL_Texture* TT[n];
	string strText[n] = {
		"Add",
		"Search",
		"Name:",
		"Number:",
		"Delit",
		"Close",
		"Exit"
	};
	for (int numStr = 0; numStr < n; ++numStr)
	{
		TT[numStr] = CreateText(strText[numStr]);
	}


	//Размеры кнопок, громозко да...
	SDL_Rect Size_addC
	{
	Screen.w / 100 * 20, Screen.h / 100 * 82, Screen.w / 100 * 60 + 20,
			Screen.h - Screen.h / 100 * 98 };

	SDL_Rect Size_findC
	{
	Screen.w / 100 * 20, Screen.h / 100 * 92, Screen.w / 100 * 60 + 20,
			Screen.h - Screen.h / 100 * 98 };

	SDL_Rect Size_closeC
	{
	Screen.w / 100 * 20, Screen.h / 100 * 92, Screen.w / 100 * 60 + 20,
			Screen.h - Screen.h / 100 * 98 };
			
	SDL_Rect Size_exitC
	{
	10, Screen.h / 100 * 82, Screen.w / 100 * 15,
			Screen.h - Screen.h / 100 * 88};
			
	SDL_Rect Size_delitC
	{
		Screen.w / 100 * 80, Screen.h / 100 * 40, Screen.w / 100 * 80 + 50,
		Screen.h - Screen.h / 100 * 99 };

	SDL_Rect Size_boksName
	{
	Screen.w / 100 * 10, Screen.h / 100 * 22, Screen.w / 100 * 80 + 50,
			Screen.h - Screen.h / 100 * 99 };

	SDL_Rect Size_boksNumber
	{
	Screen.w / 100 * 10, Screen.h / 100 * 30, Screen.w / 100 * 80 + 50,
			Screen.h - Screen.h / 100 * 99 };

	SDL_Rect Size_boksFind
	{
	Screen.w / 100 * 10, Screen.h / 100 * 22, Screen.w / 100 * 80 + 50,
			Screen.h - Screen.h / 100 * 99 };

	SDL_Rect Size_boksResult
	{
	Screen.w / 100 * 10, Screen.h / 100 * 40, Screen.w / 100 * 80 + 50,
			Screen.h - Screen.h / 100 * 99 };


	// Элементы интерфейса
	SDL_Texture *fon = nullptr;
	Button addC(TT[0], &Size_addC, 0);
	Button findC(TT[1], &Size_findC, 0);
	Button closeC(TT[5], &Size_closeC, 0);
	Button boksName(TT[2], &Size_boksName, 1);
	Button boksNumber(TT[3], &Size_boksNumber, 1);
	Button boksFind(TT[3], &Size_boksFind, 1);
	Button boksResult(resultFindT, &Size_boksResult, 1);
	Button boksDelit(TT[4], &Size_delitC, 1);
	Button Exit(TT[6], &Size_exitC, 0);

	//Загрузка фона
	fon = LoadImage("fon.jpeg");
	SDL_QueryTexture(fon, NULL, NULL, &bW, &bH);

	//Цвет интерфейсов
	SDL_SetRenderDrawColor(renderer, 0x46, 0x82, 0xF6, 0xFF);

	//Точка касания
	SDL_Point touchL;

	//Открытие книги
	ContactBook PhoneBook;
	PhoneBook.ReadContactFile();

	//Переменые текущего окна и контроля ввода
	short state = 0, input = 0;
	//Цикл ределинга
	bool done = false;
	while (!done)
	{
		//Для хранения кординат касания прошлого витка и удобства
		unsigned int X = 0-1, Y = 0-1;

		//Отчиска рендера, и передача ему фона
		SDL_RenderClear(renderer);
		ApplySurface(0, 0, fon);

		//Цикл обработки событий
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			//Cкорлинг
			case SDL_FINGERMOTION:
			{
				//Снятие кординат
				touchL.x = event.tfinger.x * SW;
				touchL.y = event.tfinger.y * SH;

				//Замедление в мс, иначе слишком быстрый
				SDL_Delay(7);

				//Проверки текущего состояния
				if (PhoneBook.Touch(X, Y) && PhoneBook.SizeMap() > 5 && state == 0)
				{
					if (Y > touchL.y && PhoneBook.GetCount() < (PhoneBook.SizeMap() - 5))
						PhoneBook.SetCount((PhoneBook.GetCount()) + 1);
					else if (Y < touchL.y && PhoneBook.GetCount() > 0)
						PhoneBook.SetCount((PhoneBook.GetCount()) - 1);
				}
				X = touchL.x;
				Y = touchL.y;
				break;
			}

			//Нажатие
			case SDL_FINGERDOWN:
			{
				touchL.x = event.tfinger.x * SW;
				touchL.y = event.tfinger.y * SH;
				X = touchL.x;
				Y = touchL.y;

				//Клавиши и действия
				//--Переход к экрану добавления
				if (addC.Touch(X, Y) && state == 0)
				{
					boksName.SetText(TT[2]);
					boksNumber.SetText(TT[3]);
					state = 1;
				}
				//--Добавить
				else if (addC.Touch(X, Y) && state == 1)
				{
					PhoneBook.Add(inputName, inputNumber);					
					inputName = "";
					inputNumber = "";
					state = 0;
				}
				//--Переход к пойску
				else if (findC.Touch(X, Y) && state == 0)
				{
					boksFind.SetText(TT[2]);
					resultFind = "";
					state = 2;
				}
					else if (Exit.Touch(X, Y) && state == 0)
				{
					done=true;
				}
				//--Выход в основное окно
				else if (closeC.Touch(X, Y) && state != 0)
				{
					state = 0;
				}
				//--Ввод имени
				else if (boksName.Touch(X, Y) && state == 1)
				{
					input = 1;
					SDL_StartTextInput();
					inputNameT = CreateText(inputName);
					boksName.SetText(inputNameT);
				}
				//--Ввод номера
				else if (boksNumber.Touch(X, Y) && state == 1)
				{
					input = 2;
					SDL_StartTextInput();
					inputNumberT = CreateText(inputNumber);
					boksNumber.SetText(inputNumberT);
				}
				//--Ввод имени при пойске
				else if (boksFind.Touch(X, Y) && state == 2)
				{
					input = 3;
					SDL_StartTextInput();
					inputFindT = CreateText(inputFind);
					boksFind.SetText(inputFindT);

				}
				//--Удаление
				else if (boksDelit.Touch(X, Y) && state == 2)
				{
					LogStream << "xx";
					PhoneBook.Delit(inputFind);
					boksFind.SetText(TT[5]);
					resultFind = "";
				}
				//--Для закрытия клавы по тапу на экран во время ввода
				else
				{
					SDL_StopTextInput();
				}
				break;
			}

			//События при вводе текста
			case SDL_TEXTINPUT:
			{
				//Имени
				if (input == 1)
				{
					inputName += event.text.text;
					inputNameT = CreateText(inputName);
					boksName.SetText(inputNameT);
				}
				//Номера
				else if (input == 2)
				{
					inputNumber += event.text.text;
					inputNumberT = CreateText(inputNumber);
					boksNumber.SetText(inputNumberT);
				}
				//Пойска, ищет во время ввода, (но пока по вхождению, поскольку нет отдельного класа GUI для записей
				else if (input == 3)
				{
					resultFind = PhoneBook.Find(inputFind);
					inputFind += event.text.text;
					inputFindT = CreateText(inputFind);
					boksFind.SetText(inputFindT);
				}
				break;

			//Обработка клавиш
			case SDL_KEYDOWN:
			{
				switch (event.key.keysym.sym)
				{
				//--Стирание символа при вводе
				case SDLK_BACKSPACE:
				{
					if (input == 1)
					{
						inputName.pop_back();
						inputNameT = CreateText(inputName);
						boksName.SetText(inputNameT);
					}
					else if (input == 2)
					{
						inputNumber.pop_back();
						inputNumberT = CreateText(inputNumber);
						boksNumber.SetText(inputNumberT);
					}
					else if (input == 3)
					{
						inputFind.pop_back();
						inputFindT = CreateText(inputFind);
						boksFind.SetText(inputFindT);
					}
					break;
				}
				//--Ентер
				case SDLK_RETURN:
				{
					SDL_StopTextInput();
					break;
				}
				}
				break;
			}
			}
			}
		}

		//Для определение текущего экрана
		switch (state)
		{

		//Главная
		case 0:
		{
			PhoneBook.Draw();
			addC.Draw();
			findC.Draw();
			Exit.Draw();
			break;
		}

		//Добавления контакта
		case 1:
		{
			boksName.Draw();
			boksNumber.Draw();
			addC.Draw();
			closeC.Draw();
			break;
		}

		//Пойск и удаление
		case 2:
		{
			boksFind.Draw();
			if (resultFind != "")
			{
				resultFindT = CreateText(resultFind);
				boksResult.SetText(resultFindT);
				resultFind = PhoneBook.Find(inputFind);

				boksResult.Draw();
				if (resultFind != "Not fount")
					boksDelit.Draw();
			}
			closeC.Draw();
			break;
		}
		}

		//Обновление рендера (Рисование)
		SDL_RenderPresent(renderer);
		SDL_Delay(100);
	}
	//Сохранение
	PhoneBook.SaveContactFile();
	//Выход, и приборка
	SDL_DestroyTexture(inputNameT);
	SDL_DestroyTexture(inputNumberT);
	SDL_DestroyTexture(inputFindT);
	SDL_DestroyTexture(resultFindT);

	for (int numStr = 0; numStr < strText->size(); ++numStr)
	{
		SDL_DestroyTexture(TT[numStr]);
	}
	SDL_DestroyTexture(fon);
	CloseP();
	return 0;
}


//Реализация методов
void ContactBook::Add(string & Name, string & Number)
{
	Blotter_Map[Name] = Number;
}

void ContactBook::Delit(string & Name)
{
	Blotter_Map.erase(Name);
}

string ContactBook::Find(string & Name)
{
	string ret = "";
	if (Name != "Name:" && Name != "")
	{
		iter = Blotter_Map.find(Name);
		if (iter != Blotter_Map.end())
		{
			ret = iter->first;
			ret += "    ";
			ret += iter->second;
		}
		else
		{
			ret = "Not fount";
		}
	}
	return ret;
}

void ContactBook::ReadContactFile()
{
	ifstream Read("Contact.txt");
	if (Read.is_open())
	{
		string Name, Number;
		while (!Read.eof())
		{
			Read >> Name;
			Read >> Number;
			Blotter_Map[Name] = Number;
		}
	}
	Read.close();
}

void ContactBook::SaveContactFile()
{
	ofstream Write("Contact.txt");
	for (auto it = Blotter_Map.begin(); it != Blotter_Map.end(); it++)
	{
		Write << it->first << "\t\t" << it->second << endl;
	}
	Write.close();
}

void ContactBook::Draw()
{
	if (Blotter_Map.empty())
		return;
	int NameA, NameB, NumA, NumB, i = 1, temp;
	temp = count;
	//Переход к текущей верхней записи
	while (temp != 0)
	{
		++iter;
		temp--;
	}
	for (; (iter != Blotter_Map.end()) && (i <= 6); ++iter, ++i)
	{
		SDL_RenderFillRect(renderer, &RecordingSize);

		//Отцентровка текста
		NumA = (RecordingSize.w) / 2 + RecordingSize.x;
		NumB = (RecordingSize.h) / 2 + RecordingSize.y;
		NumA = (NumA / 2) + RecordingSize.x;
		NumB = (NumB - RecordingSize.y) / 2 + RecordingSize.y;

		NameB = (RecordingSize.h) / 2 + RecordingSize.y;
		NameB = (NameB - RecordingSize.y) / 2 + RecordingSize.y;
		NameA = RecordingSize.x + 15;

		textNameT = CreateText(iter->first);
		textNumberT = CreateText(iter->second);

		ApplySurface(NameA, NameB, textNameT);
		ApplySurface(NumA, NumB, textNumberT);
		
		//Изменение размера
		RecordingSize.y = Screen.h / 100 * 7 * i+ 60 * (i- 1);
	}
	iter = Blotter_Map.begin();
}

bool ContactBook::Touch(int x, int y)
{
	if (y < Screen.h / 100 * 70)
		return true;
	return false;
}


//Уже кнопки
Button::Button(SDL_Texture * bText, SDL_Rect * size, int type)
{
	buttonText = bText;

	ButtonSize.x = size->x;
	ButtonSize.y = size->y;
	ButtonSize.w = size->w;
	ButtonSize.h = size->h;

	typeButton = type;
}

void Button::Draw()
{
	SDL_RenderFillRect(renderer, &ButtonSize);
	int TextA = 0, TextB = 0;
	//Положение текста зависит от типа кнопи(кнопка/поле)
	if (typeButton == 0)
	{
		TextA = (ButtonSize.w) / 2 + ButtonSize.x;
		TextB = (ButtonSize.h) / 2 + ButtonSize.y;
		TextA = (TextA / 2) + ButtonSize.x;
		TextB = (TextB - ButtonSize.y) / 2 + ButtonSize.y;
	}
	else
	{
		TextB = (ButtonSize.h) / 2 + ButtonSize.y;
		TextB = (TextB - ButtonSize.y) / 2 + ButtonSize.y;
		TextA = ButtonSize.x + 15;
	}
	ApplySurface(TextA, TextB, buttonText);
}

void Button::SetText(SDL_Texture * bText)
{
	buttonText = bText;
}

bool Button::Touch(int x, int y)
{
	if (x > ButtonSize.x && y > ButtonSize.y && x < (ButtonSize.w + ButtonSize.x)
		&& y < (ButtonSize.h + ButtonSize.y))
		return true;
	return false;
}



//GUI
bool Init_SDL()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		LogStream << "\nОшибка инициализации:\n" << SDL_GetError() << endl;
		return false;
	}
	return true;
}

bool Init_ComponentSDL()
{
	win = SDL_CreateWindow("Телефоная книга", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, Screen.w, Screen.h, SDL_WINDOW_SHOWN);

	renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	IMG_Init(IMG_INIT_JPG);
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();

	if (win == nullptr || renderer == nullptr)
	{
		LogStream << "\nОшибка инициализации:\n" << SDL_GetError() << endl;
		return false;
	}
	return true;
}

SDL_Texture *LoadImage(string file)
{
	SDL_Surface *Image = nullptr;
	SDL_Texture *texture = nullptr;
	Image = IMG_Load(file.c_str());
	if (Image != nullptr)
	{
		texture = SDL_CreateTextureFromSurface(renderer, Image);
		SDL_FreeSurface(Image);
	}
	else
	{
		LogStream << "\nОшибка текстур:\n" << SDL_GetError() << endl <<
			IMG_GetError() << endl;
	}
	return texture;
}

SDL_Texture *CreateText(string text)
{
	SDL_Color ColorT{220, 227, 220 };
	SDL_Surface *textS = nullptr;
	SDL_Texture *textT = nullptr;
	textS = TTF_RenderText_Solid(font, text.c_str(), ColorT);
	textT = SDL_CreateTextureFromSurface(renderer, textS);
	if (textT == nullptr || textS == nullptr)
	{
		LogStream << "Ошибка работы с текстом:\n" << TTF_GetError() << endl;
	}
	return textT;
}

void ApplySurface(int x, int y, SDL_Texture * tex)
{
	SDL_Rect pos;
	pos.x = x;
	pos.y = y;

	SDL_QueryTexture(tex, NULL, NULL, &pos.w, &pos.h);
	SDL_RenderCopy(renderer, tex, NULL, &pos);
}

void CloseP()
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(win);
	TTF_CloseFont(font);
	TTF_Quit();
	SDL_Quit();
}