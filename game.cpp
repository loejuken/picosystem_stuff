#include "game.hpp"
#include <vector>
#include <algorithm>
#include <random>
#include <ctime>

using namespace blit;

enum class GameState { DEAL, PLAYER_TURN, DEALER_TURN, GAME_OVER };

struct Card {
    int value; // 1 to 11
    std::string name; // "A", "2", ..., "K"
};

std::vector<Card> deck;
std::vector<Card> player_hand;
std::vector<Card> dealer_hand;

GameState game_state = GameState::DEAL;
bool show_result = false;
std::string result_text;

int player_score = 0;
int dealer_score = 0;

void shuffle_deck() {
    deck.clear();
    std::vector<std::string> names = {"A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};
    for (const auto &name : names) {
        int val = name == "A" ? 11 : (name == "J" || name == "Q" || name == "K" ? 10 : std::stoi(name));
        for (int i = 0; i < 4; i++) { // 4 suits
            deck.push_back({val, name});
        }
    }
    std::shuffle(deck.begin(), deck.end(), std::default_random_engine(time(NULL)));
}

int calculate_score(const std::vector<Card>& hand) {
    int score = 0;
    int aces = 0;
    for (const auto& card : hand) {
        score += card.value;
        if (card.name == "A") aces++;
    }
    while (score > 21 && aces--) score -= 10;
    return score;
}

void deal_initial_cards() {
    player_hand.clear();
    dealer_hand.clear();
    player_hand.push_back(deck.back()); deck.pop_back();
    dealer_hand.push_back(deck.back()); deck.pop_back();
    player_hand.push_back(deck.back()); deck.pop_back();
    dealer_hand.push_back(deck.back()); deck.pop_back();
    player_score = calculate_score(player_hand);
    dealer_score = calculate_score(dealer_hand);
}

void init() {
    set_screen_mode(ScreenMode::hires);
    shuffle_deck();
    deal_initial_cards();
    game_state = GameState::PLAYER_TURN;
}

void render(uint32_t time) {
    screen.pen = Pen(0, 0, 0);
    screen.clear();
    screen.pen = Pen(255, 255, 255);
    
    int y = 10;
    screen.text("Player Hand:", minimal_font, Point(10, y));
    y += 12;
    for (const auto& card : player_hand) {
        screen.text(card.name + " ", minimal_font, Point(10, y));
        y += 12;
    }

    y += 10;
    screen.text("Dealer Hand:", minimal_font, Point(10, y));
    y += 12;
    for (const auto& card : dealer_hand) {
        screen.text(card.name + " ", minimal_font, Point(10, y));
        y += 12;
    }

    y += 10;
    screen.text("Player Score: " + std::to_string(player_score), minimal_font, Point(10, y));
    y += 12;
    if (game_state == GameState::GAME_OVER) {
        screen.text("Dealer Score: " + std::to_string(dealer_score), minimal_font, Point(10, y));
        y += 12;
        screen.text(result_text, minimal_font, Point(10, y));
    }
}

void update(uint32_t time) {
    if (game_state == GameState::PLAYER_TURN) {
        if (buttons & Button::A) { // Hit
            player_hand.push_back(deck.back()); deck.pop_back();
            player_score = calculate_score(player_hand);
            if (player_score > 21) {
                game_state = GameState::GAME_OVER;
                result_text = "Player Busts! Dealer Wins.";
                dealer_score = calculate_score(dealer_hand);
            }
        }
        if (buttons & Button::B) { // Stand
            game_state = GameState::DEALER_TURN;
        }
    } else if (game_state == GameState::DEALER_TURN) {
        dealer_score = calculate_score(dealer_hand);
        while (dealer_score < 17) {
            dealer_hand.push_back(deck.back()); deck.pop_back();
