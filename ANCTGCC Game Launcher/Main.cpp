# include <Siv3D.hpp>

/* Window parameter */
const String launcher_title = L"GCC 2016 Launcher";
const Size window_size(800, 600);

/* UI parameter */
const int thumbnail_width = 50;
const int interval = 200;
const Point icon_offset(thumbnail_width + interval, 50);
const Rect icon_rect(icon_offset, 300, 300);
const Color msgbox_color = Color(Palette::Lightblue).setAlpha(120);
const Vec2 title_offset(10, 10);
const Vec2 msg_offset(10, 55);
const Color credit_color = Color(Palette::Whitesmoke).setAlpha(245);
const Rect enter_zone = icon_rect.stretched(icon_offset.y, 0, 0, 0);
//const Point up_arrow_position();

const int transit_speed = 50;
const int background_brighten_speed = 2;

/* Game info parameter*/
const String game_0_title = L"ＧＣＣの世界から外に出ます。";
const String game_1_title = L"ＧＣＣの世界へようこそ！";

struct Game
{
	const TextureRegion icon;
	const TextureRegion background;
	String message;
	FilePath exe_path;
	String title;

	Game(FilePath path)
		: message(TextReader::TextReader(path + L"message.txt").readAll())
		, background(Texture(path + L"background.png").resize(Window::Size().yy()))
		, icon(Texture(path + L"icon.png").resize(icon_rect.size))
	{
		auto app_dir = path + L"Game/";
		if (FileSystem::IsDirectory(app_dir)) {
			auto pathes = FileSystem::DirectoryContents(app_dir);
			const auto& p = std::find_if(pathes.cbegin(), pathes.cend(), [](const auto& p) { return p.includes(L".exe"); });
			if (p != pathes.cend()) {
				exe_path = *p;
				title = FileSystem::BaseName(*p);
			}
		}
	}
};

void Main()
{
	const Rect msgbox_rect(120, 400, 560, 150);

	Window::SetTitle(launcher_title);
	Window::Resize(window_size);

	const Size arrow_size(100, 100);

	const Font title(18, L"/64604");
	const Font font(10, L"/64604");
	const TextureRegion arrow = Texture(L"/200").resize(arrow_size);
	const TextureRegion arrow_forward = Texture(L"/201").resize(arrow_size);
	const String credits = TextReader(L"Games/01 GCC/credits.txt").readAll();

	const int max_alpha = 255 / 2;
	int alpha = 0;
	Color background_cover_color(Palette::White);

	std::vector<Point> icon_positions;
	std::vector<Game> games;

	/* Load Game info */ {
		Array<FilePath> contents = FileSystem::DirectoryContents(L"Games");
		std::sort(contents.begin(), contents.end());

		const int icon_interval = icon_rect.size.x + interval;

		for (auto i = 0u; i < contents.size(); i++) {
			games.push_back(Game(std::move(contents[i])));
			icon_positions.push_back(Point(icon_offset).movedBy(icon_interval * i, 0));
		}

		games[0].title = game_0_title;
		games[1].title = game_1_title;
	}

	int camera_position = -window_size.x;
	int cursored_index = 1;
	while (System::Update())
	{
		/* Update section */ {
			if (camera_position == icon_positions[cursored_index].x) {
				if (Input::KeyEnter.clicked || Input::KeyUp.clicked || Input::MouseL.clicked && enter_zone.contains(Mouse::Pos())) {
					if (cursored_index == 0) {
						break;
					}
					else if (cursored_index == 1) {
						do {
							games[1].background.drawAt(Window::Size() / 2, credit_color);
							font(credits).draw(120, 20, Palette::Black);
						} while (System::Update() && !Input::AnyKeyClicked());
					}
					else {
						Optional<ProcessInfo> process = System::CreateProcess(games[cursored_index].exe_path);
						while (process->isRunning()) {
							Window::Minimize();
						}
						Window::Restore();
					}
				}
			}
			if (Input::KeyLeft.clicked || Input::MouseL.clicked && Mouse::Pos().x < 200) {
				cursored_index = Max(cursored_index - 1, 0);
			}
			if (Input::KeyRight.clicked || Input::MouseL.clicked && Mouse::Pos().x > 600) {
				cursored_index = Min(cursored_index + 1, static_cast<int>(games.size() - 1));
			}
			if (camera_position - icon_positions[cursored_index].x > 0) {
				alpha = 0;
				camera_position -= transit_speed;
			}
			if (camera_position - icon_positions[cursored_index].x < 0) {
				alpha = 0;
				camera_position += transit_speed;
			}

			alpha = Min(alpha + background_brighten_speed, max_alpha);
			if (cursored_index == 0 || cursored_index == 1) {
				alpha = max_alpha;
			}
		}
		/* Draw section */ {
			games[cursored_index].background.draw(100, 0, background_cover_color.setAlpha(alpha));
			for (auto i = Max(0, cursored_index - 1); i < Min((int)games.size(), cursored_index + 1); i++) {
				games[i].icon.draw(icon_positions[i].movedBy(icon_offset.x - camera_position, 0));
			}

			msgbox_rect.draw(msgbox_color);
			title(games[cursored_index].title).draw(msgbox_rect.tl + title_offset);
			font(games[cursored_index].message).draw(msgbox_rect.tl + msg_offset);

			if (cursored_index != 0) {
				arrow.drawAt(150, 200);
			}
			if (cursored_index != static_cast<int>(games.size()) - 1) {
				arrow.rotate(Math::Pi).drawAt(650, 200);
			}
			arrow_forward.drawAt(400, 50);
		}
	}
}
