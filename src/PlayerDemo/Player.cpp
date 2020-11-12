#include "Player.h"

Player::Player(QObject *parent) : QObject(parent)
{
    m_player.Start();

}

Player::~Player()
{
    m_player.Close();
}


bool Player::SetCurrentMedia(QString url,double progess)
{
    m_player.SetCurrentMedia(url.toLocal8Bit(),&m_d3dWidget);
    return true;
}

void Player::SetNextMedia(QString url)
{
    m_player.SetNextMedia(url);
}

void Player::SetPause(bool isPause)
{
    m_player.SetPause(isPause);
}

bool Player::IsPause()
{
    return m_player.IsPause();
}

void Player::Seek(double progress)
{
    m_player.Seek(progress);
}

long long Player::GetDuration()
{
    return m_player.GetDuration();
}

void Player::SetVolume(double volume)
{
    m_player.SetVolume(volume);
}

void Player::SetViewportRect(QVector<QRect> &rects)
{
    m_player.SetViewportRect(rects);
}

long long Player::GetCurrentTime()
{
    return m_player.GetCurrentTime();
}











