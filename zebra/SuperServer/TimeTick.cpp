/**
 * \brief 时间回调函数
 *
 * 
 */

#include "SuperServer.h"

zRTime SuperTimeTick::currentTime;
SuperTimeTick *SuperTimeTick::instance = NULL;

void SuperTimeTick::run()
{
  readTime();

  while(!isFinal())
  {
    zThread::sleep(1);

    currentTime.now();
    Zebra::qwGameTime = qwStartGameTime + startTime.elapse(currentTime) / 1000;
    
    if (0 != Zebra::qwGameTime
        && 0 == Zebra::qwGameTime % 600)
    {
      saveTime();
    }

    Cmd::Super::t_GameTime tCmd;
    tCmd.qwGameTime = Zebra::qwGameTime;
    ServerManager::getInstance().broadcast(&tCmd,sizeof(tCmd));
  }

  saveTime();
}

static const dbCol gametime_define[] =
{
  {"`GAMETIME`",zDBConnPool::DB_QWORD,sizeof(QWORD)},
  {NULL,0,0}
};

bool SuperTimeTick::readTime()
{
  connHandleID handle = SuperService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("不能从数据库连接池获取连接句柄");
    return false;
  }

  DWORD retcode = SuperService::dbConnPool->exeSelectLimit(handle,"`GAMETIME`",gametime_define,NULL,NULL,1,(BYTE*)(&qwStartGameTime));
  if ((DWORD)-1 == retcode)
  {
    SuperService::dbConnPool->putHandle(handle);
    Zebra::logger->error("读取游戏时间错误");
    return false;
  }

  if (0 == retcode)
  {
    qwStartGameTime = 0;
    retcode = SuperService::dbConnPool->exeInsert(handle,"`GAMETIME`",gametime_define,(const BYTE *)(&qwStartGameTime));
    if ((DWORD)-1 == retcode)
    {
      SuperService::dbConnPool->putHandle(handle);
      Zebra::logger->error("初始化游戏时间错误");
      return false;
    }
  }
  SuperService::dbConnPool->putHandle(handle);

  Zebra::logger->debug("读取游戏时间成功");
  Zebra::qwGameTime = qwStartGameTime;
  startTime.now();

  return true;
}

bool SuperTimeTick::saveTime()
{
  connHandleID handle = SuperService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("不能从数据库连接池获取连接句柄");
    return false;
  }

  DWORD retcode = SuperService::dbConnPool->exeUpdate(handle,"`GAMETIME`",gametime_define,(BYTE*)(&Zebra::qwGameTime),NULL);
  SuperService::dbConnPool->putHandle(handle);

  if (1 == retcode)
  {
    Zebra::logger->debug("保存游戏时间成功");
  }
  else
  {
    Zebra::logger->error("保存游戏时间失败");
  }

  return true;
}

