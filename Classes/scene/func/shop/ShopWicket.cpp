#include "ShopWicket.h"

#include "common/utils/TimeUtil.h"
#include "scene/gui/ModalDialog.h"
#include "scene/SoundMgr.h"
#include "game/gamecenter/GameCenter.h"
#include "game/firservice/FirebaseService.h"
#include "game/nts/TimeService.h"
#include "game/UserPreferences.h"

using namespace cocos2d::ui;

NS_BEGIN

#define FRAME_HEIGHT					233
#define FRAME_MAX_WIDTH					458
#define FRAME_MARGIN_LEFT_RIGHT			25

#define ITEM_WIDTH						124		

ShopWicket* ShopWicket::create(Node* owner)
{
	ShopWicket *ret = new (std::nothrow) ShopWicket();
	if (ret && ret->init(owner))
	{
		ret->autorelease();
		return ret;
	}
	else
	{
		CC_SAFE_DELETE(ret);
		return nullptr;
	}
}

bool ShopWicket::init(Node* owner)
{
	auto visibleSize = Director::getInstance()->getVisibleSize();
	float frameWidth = visibleSize.width - FRAME_MARGIN_LEFT_RIGHT * 2;
	frameWidth = MIN(frameWidth, FRAME_MAX_WIDTH);

	if (!Wicket::init(owner, Size(frameWidth, FRAME_HEIGHT), sLocaleMgr->getString("shop_wicket_title"), true, false))
		return false;

	sStore->addListener(this);
	sAdManager->addRewardedAdListener(RewardedAdConfig::AD_TYPE_FREE_GOLDS, this);
    sTimeService->addListener(this);

	m_main = Node::create();
	this->setContent(m_main);

	Size frameSize = m_main->getBoundingBox().size;

	m_listView = ListView::create();
	m_listView->setClippingEnabled(true);
	m_listView->setSwallowTouches(true);
	m_listView->setScrollBarEnabled(false);
	m_listView->setDirection(ui::ScrollView::Direction::HORIZONTAL);
	m_listView->setBounceEnabled(true);
	m_listView->setContentSize(Size(frameSize.width, frameSize.height));
	m_listView->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
	m_listView->setPosition(Vec2(0, 0));
	m_listView->addEventListener((ui::ListView::ccListViewCallback)CC_CALLBACK_2(ShopWicket::selectedItemEvent, this));
	m_main->addChild(m_listView);

	return true;
}

ShopWicket::ShopWicket() :
	m_main(nullptr),
	m_listView(nullptr),
	m_loadingView(nullptr),
	m_isWatchAdHintEnabled(false),
    m_isRequestingRewardedVideo(false)
{

}

ShopWicket::~ShopWicket()
{
	sStore->removeListener(this);
	sAdManager->removeRewardedAdListener(this);
    sTimeService->removeListener(this);

	m_main = nullptr;
	m_listView = nullptr;
	m_loadingView = nullptr;
}

void ShopWicket::onClosed()
{
    GameCoinRewardItem* rewardItem = dynamic_cast<GameCoinRewardItem*>(this->getItemByGameCoinId(GAME_COIN_FREE_GOLDS));
    if(rewardItem)
    {
        if(!rewardItem->isTimeToWatchAd())
        {
        	if(rewardItem->isTimerEnabled())
				this->saveSuggestionAccepted(SUGGESTION_WATCH_AD_REMINDED, false);
        }
        else
			this->saveSuggestionAccepted(SUGGESTION_WATCH_AD_REMINDED, true);
    }
}

void ShopWicket::onEnter()
{
    Wicket::onEnter();

    this->loadData();
}

void ShopWicket::loadData()
{
	if (!sStore->isProductIdsValidated() && !sStore->isValidatingProductIds())
		this->validateProductIds();
    
    if(sTimeService->getTimeState() == TimeService::TIME_NONE)
        sTimeService->syncTime();

	auto tmplList = sShopMgr->getGameCoinTemplateList();
	for (size_t i = 0; i < tmplList->size(); ++i)
	{
		auto const& tmpl = (*tmplList)[i];
		GameCoinItem* item;
		if (tmpl.productType == PRODUCT_TYPE_PRIZE)
		{
			GameCoinRewardItem* rewardItem = GameCoinRewardItem::create(Size(ITEM_WIDTH, m_listView->getContentSize().height));
			rewardItem->setWatchAdHintEnabled(m_isWatchAdHintEnabled);
			item = rewardItem;
		}
		else
			item = GameCoinSaleItem::create(Size(ITEM_WIDTH, m_listView->getContentSize().height));
		item->setData(&tmpl);
		if (i == tmplList->size() - 1)
			item->setLastItem(true);
		m_listView->pushBackCustomItem(item);
	}
}

void ShopWicket::updateListItems()
{
	Vector<Widget*>& items = m_listView->getItems();
	for (ssize_t i = 0; i < items.size(); ++i)
	{
		GameCoinItem* item = dynamic_cast<GameCoinItem*>(items.at(i));
		if (item)
		{
			item->reloadData();
			item->setLastItem(i == items.size() - 1);
		}
	}
}

void ShopWicket::reloadGameCoinSaleItems()
{
    Vector<Widget*>& items = m_listView->getItems();
    for (ssize_t i = 0; i < items.size(); ++i)
    {
        GameCoinSaleItem* item = dynamic_cast<GameCoinSaleItem*>(items.at(i));
        if (item)
            item->reloadData();
    }
}

GameCoinItem* ShopWicket::getItemByGameCoinId(GameCoinID gameCoinId)
{
    Vector<Widget*>& items = m_listView->getItems();
    auto it = std::find_if(items.begin(), items.end(), [gameCoinId](Widget* widget)->bool{
        GameCoinItem* item = dynamic_cast<GameCoinItem*>(widget);
        if(item)
            return item->getData()->id == gameCoinId;
        return false;
    });
    if(it != items.end())
        return dynamic_cast<GameCoinItem*>(*it);
    
    return nullptr;
}

void ShopWicket::giveReward(GameCoinTemplate const* tmpl)
{
	LocalPlayer* localPlayer = sGameCenter->getLocalPlayer();
	int32 stage = localPlayer->getRewardStage();
    stage = std::min(stage, (int32)(tmpl->stageRewardList.size() - 1));
	StageGameCoin const& gameCoin = tmpl->stageRewardList[stage];
	localPlayer->addMoney(gameCoin.amount);

	uint8 nextStage = stage + 1;
	if (nextStage >= tmpl->stageRewardList.size())
		nextStage = (uint8)(tmpl->stageRewardList.size() - 1);
	localPlayer->setRewardStage(nextStage);

	StageGameCoin nextStageGameCoin = tmpl->stageRewardList[nextStage];
	uint32 currTime = (uint32)((sTimeService->getCurrentTimeMillis() + 999) / 1000);
	uint32 nextRewardTime = (uint32)(currTime + nextStageGameCoin.waitTime * 60);
	localPlayer->setRewardTime(nextRewardTime);

	localPlayer->saveDataAsync();
}

void ShopWicket::deliverGameCoin(GameCoinTemplate const* tmpl, std::function<void(GameCoinItem*)> const& deliveredCallback)
{
	Vector<Widget*>& items = m_listView->getItems();
	for (ssize_t i = 0; i < items.size(); ++i)
	{
		GameCoinItem* item = dynamic_cast<GameCoinItem*>(items.at(i));
		if (item)
		{
			if (item->getData()->id == tmpl->id)
			{
			    sSoundMgr->play(SOUND_DELIVER_COINS);
				this->playDeliveryAnimation(item, [deliveredCallback, item](){
					if(deliveredCallback)
						deliveredCallback(item);
				});
				break;
			}
		}
	}
}

void ShopWicket::buyGameCoin(GameCoinTemplate const* tmpl)
{
	if (!sStore->canMakePayments())
	{
		MessageDialog* dialog = MessageDialog::create();
		dialog->setTitle(sLocaleMgr->getString("store_dlg_title_buy_fail"));
		dialog->addOkButton(nullptr);
		dialog->setMessage(sLocaleMgr->getString("store_dlg_msg_not_allowed_make_payments"));
		dialog->show();
	}
	else
	{
		if (!sStore->hasDeferredTransaction(tmpl->productId))
		{
			this->scheduleOnce([this, tmpl](float dt){
				sStore->requestPayment(tmpl->productId);
			}, 0.1f, "RequestPayment");
		}
		else
		{

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
            MessageDialog* dialog = MessageDialog::create();
			dialog->setTitle(sLocaleMgr->getString("store_dlg_title_buy_deferred"));
            dialog->addOkButton(nullptr);
            dialog->setMessage(sLocaleMgr->getString("store_dlg_msg_pending_purchase"));
            dialog->show();
#elif CC_TARGET_PLATFORM == CC_PLATFORM_IOS
            MessageDialog* dialog = MessageDialog::create();
			dialog->setTitle(sLocaleMgr->getString("store_dlg_title_buy_deferred"));
            dialog->addOkButton(nullptr);
            dialog->setMessage(sLocaleMgr->getString("store_dlg_msg_pending_approval"));
            dialog->show();
#endif

		}
	}
}

void ShopWicket::playDeliveryAnimation(GameCoinItem* item, std::function<void()> const& complete)
{
	Sprite* picSp = item->clonePicture();
	Point screenPos = picSp->convertToWorldSpace(Vec2(picSp->getContentSize().width * picSp->getAnchorPoint().x, picSp->getContentSize().height * picSp->getAnchorPoint().y));
	Point localPos = this->convertToNodeSpace(screenPos);
	picSp->setPosition(localPos);
	picSp->removeFromParent();
	this->addChild(picSp);
	auto callFunc = CallFunc::create([this, complete, item]() {
		m_listView->resume();
		this->getCloseButton()->setTouchEnabled(true);
		item->setTouchEnabled(true);
		if (complete)
			complete();
	});
	m_listView->pause();
	this->getCloseButton()->setTouchEnabled(false);
	item->setTouchEnabled(false);
	auto moveAndScale1 = EaseCircleActionOut::create(Spawn::createWithTwoActions(MoveBy::create(0.3f, Vec2(0, -20)), ScaleBy::create(0.3f, 1.3f)));
	auto moveAndScale2 = EaseCircleActionIn::create(Spawn::createWithTwoActions(MoveTo::create(0.8f, m_goldDeliveryPoint), ScaleBy::create(0.8f, 0.4f)));
	auto action = Sequence::create(moveAndScale1, moveAndScale2, RemoveSelf::create(), callFunc, nullptr);
	picSp->runAction(action);
}

void ShopWicket::watchRewardedVideo(GameCoinTemplate const* tmpl)
{
	if(sAdManager->isRewardedVideoInProgress())
		return;

    this->requestRewardedVideo(tmpl);
}

void ShopWicket::requestRewardedVideo(GameCoinTemplate const* tmpl)
{
    if(m_isRequestingRewardedVideo)
        return;

    RewardedAdConfig adConfig;
	adConfig.adType = RewardedAdConfig::AD_TYPE_FREE_GOLDS;
	adConfig.value1 = tmpl->id;
	m_rewardedAdConfig = adConfig;

    if(sTimeService->getTimeState() == TimeService::TIME_SYNCED)
    {
        if(sAdManager->requestShowRewardedVideo(m_rewardedAdConfig))
        {
			m_isRequestingRewardedVideo = true;
            this->showLoadingView();
        }
    }
    else
    {
        this->showLoadingView();
		m_isRequestingRewardedVideo = true;
        if (sTimeService->getTimeState() == TimeService::TIME_NONE)
            sTimeService->syncTime();
    }
}

void ShopWicket::showLoadingView()
{
	if (!m_loadingView)
	{
		m_loadingView = LoadingView::create(this);
		m_loadingView->show();
	}
}

void ShopWicket::dismissLoadingView()
{
	if (m_loadingView)
	{
		m_loadingView->dismiss();
		m_loadingView = nullptr;
	}
}

void ShopWicket::onRewardedAdClosed(RewardedAdConfig const& adConfig, bool isEarnedReward)
{
    m_isRequestingRewardedVideo = false;
	this->dismissLoadingView();
}

void ShopWicket::onUserEarnedReward(RewardedAdConfig const& adConfig)
{
    GameCoinID gameCoinId = (GameCoinID)adConfig.value1.asInt();
    GameCoinTemplate const* tmpl = sShopMgr->getGameCoinTemplateByGameCoinId(gameCoinId);
    
	this->deliverGameCoin(tmpl, [this](GameCoinItem* item) {
		this->giveReward(item->getData());
		item->reloadData();

		if (m_itemStateChangedCallback)
			m_itemStateChangedCallback(this, SHOP_ITEM_STATE_REWARDED);
	});
}

void ShopWicket::onRewardedAdRequestFailed(RewardedAdConfig const& adConfig, AdError const& error)
{
	m_isRequestingRewardedVideo = false;
	this->dismissLoadingView();

	if(error.errorCode != AD_ERROR_NONE)
	{
		MessageDialog* dialog = MessageDialog::create();
        dialog->setTitle(sLocaleMgr->getString("loadad_dlg_title_fail"));
        switch (error.errorCode) 
        {
            case AD_ERROR_OS_VERSION_TOO_LOW:
                dialog->setMessage(sLocaleMgr->getString("loadad_dlg_msg_os_version_too_low"));
                break;
            default:
                dialog->setMessage(sLocaleMgr->getString("loadad_dlg_msg_fail"));
                break;
        }
		dialog->show();
	}
}

void ShopWicket::selectedItemEvent(Ref* pSender, ui::ListView::EventType type)
{
	if (type == ListView::EventType::ON_SELECTED_ITEM_END)
	{
		ListView* listView = static_cast<ListView*>(pSender);
		auto& items = listView->getItems();
		ssize_t index = listView->getCurSelectedIndex();
		if (index >= 0 && index < items.size())
		{
			GameCoinItem* item = dynamic_cast<GameCoinItem*>(items.at(index));
			if (item)
			{
				GameCoinTemplate const* data = item->getData();
				if (data->productType == PRODUCT_TYPE_PRIZE)
				{
					GameCoinRewardItem* rewardItem = dynamic_cast<GameCoinRewardItem*>(item);
					if (rewardItem->isTimeToWatchAd())
						this->watchRewardedVideo(data);
				}
				else
					this->buyGameCoin(data);
			}
		}
	}
}

void ShopWicket::validateProductIds()
{
	std::vector<std::string> productIds = sShopMgr->getProductIdList();
	sStore->validateProductIds(productIds);
}

void ShopWicket::onProductsRequestFinished()
{
	this->reloadGameCoinSaleItems();
}

void ShopWicket::onProductsRequestFailed(StoreError const& error)
{
	this->reloadGameCoinSaleItems();
}

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID

void ShopWicket::onConsumeFinished(PaymentTransaction const& transaction)
{
    IAPItem const* item = sShopMgr->getIAPItem(transaction.productId);
    switch (item->templateType)
    {
    case TEMPLATE_TYPE_GAME_COIN:
    {
        GameCoinTemplate const* tmpl = sShopMgr->getGameCoinTemplateByProductId(transaction.productId);
		this->deliverGameCoin(tmpl, [this](GameCoinItem*) {
			if (m_itemStateChangedCallback)
				m_itemStateChangedCallback(this, SHOP_ITEM_STATE_PURCHASED);
		});
        break;
    }
    default:
        break;
    }
}

void ShopWicket::onConsumeFailed(StoreError const& error)
{
}

#elif CC_TARGET_PLATFORM == CC_PLATFORM_IOS

void ShopWicket::onPaymentTransactionUpdated(PaymentTransaction const& transaction)
{
    if(transaction.state == PAYMENT_STATE_PURCHASED)
    {
        IAPItem const* item = sShopMgr->getIAPItem(transaction.productId);
        switch (item->templateType)
        {
        case TEMPLATE_TYPE_GAME_COIN:
        {
            GameCoinTemplate const* tmpl = sShopMgr->getGameCoinTemplateByProductId(transaction.productId);
			this->deliverGameCoin(tmpl, [this](GameCoinItem*) {
				if (m_itemStateChangedCallback)
					m_itemStateChangedCallback(this, SHOP_ITEM_STATE_PURCHASED);
			});
            break;
        }
        default:
            break;
        }

    }
}

#endif

void ShopWicket::onTimeSyncSuccess(int64 millis)
{
    if(m_isRequestingRewardedVideo)
    {
        if(!sAdManager->requestShowRewardedVideo(m_rewardedAdConfig))
        {
			m_isRequestingRewardedVideo = false;
            this->dismissLoadingView();
        }
    }
    else
    {
        GameCoinItem* item = this->getItemByGameCoinId(GAME_COIN_FREE_GOLDS);
        if(item)
            item->reloadData();
    }
}

void ShopWicket::onTimeSyncFail()
{
    if(m_isRequestingRewardedVideo)
    {
        this->dismissLoadingView();
		m_isRequestingRewardedVideo = false;
        
        MessageDialog* dialog = MessageDialog::create();
        dialog->setTitle(sLocaleMgr->getString("loadad_dlg_title_fail"));
        dialog->setMessage(sLocaleMgr->getString("loadad_dlg_msg_network_error"));
        dialog->show();
    }
    else
    {
        GameCoinItem* item = this->getItemByGameCoinId(GAME_COIN_FREE_GOLDS);
        if(item)
            item->reloadData();
    }
}

void ShopWicket::saveSuggestionAccepted(uint32 suggestion, bool accepted)
{
	auto localPlayer = sGameCenter->getLocalPlayer();
	if (accepted)
	{
		if (!localPlayer->isSuggestionAccepted(suggestion))
		{
			localPlayer->setSuggestionAccepted(suggestion, true);
			localPlayer->saveDataAsync();
		}
	}
	else
	{
		if (localPlayer->isSuggestionAccepted(suggestion))
		{
			localPlayer->setSuggestionAccepted(suggestion, false);
			localPlayer->saveDataAsync();
		}
	}
}


NS_END
