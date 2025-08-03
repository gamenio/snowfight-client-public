#include "Share.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)

NS_BEGIN

void Share::share(ShareInfo const& info, Vec2 const& popoverAnchor) {
    NSString *title = [NSString stringWithUTF8String: info.title.c_str()];
    NSString *text = [NSString stringWithUTF8String: info.text.c_str()];
    NSString *link = [NSString stringWithUTF8String:info.link.c_str()];
    NSURL *url = [NSURL URLWithString: link];
    NSArray *activityItems = @[text, url];
    NSMutableArray *excludedActivityTypes = [NSMutableArray arrayWithObjects:
                                             UIActivityTypePrint,
                                             UIActivityTypeAssignToContact,
                                             UIActivityTypeSaveToCameraRoll,
                                             UIActivityTypeAddToReadingList,
                                             //UIActivityTypeAirDrop,
                                             @"com.apple.mobilenotes.SharingExtension",
                                             @"com.apple.reminders.RemindersEditorExtension",
                                             @"com.apple.UIKit.activity.OpenInIBooks",
                                             @"com.apple.UIKit.activity.MarkupAsPDF",
                                             nil];

    UIActivityViewController *activityViewController = [[UIActivityViewController alloc] initWithActivityItems:activityItems applicationActivities:nil];
    activityViewController.excludedActivityTypes = excludedActivityTypes;
    [activityViewController setValue: title forKey: @"subject"];
    UIViewController* rootViewController = [UIApplication sharedApplication].keyWindow.rootViewController;
    
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad) {
        if ( [activityViewController respondsToSelector:@selector(popoverPresentationController)] ) {
            UIPopoverPresentationController *ppc = activityViewController.popoverPresentationController;
            ppc.sourceView = rootViewController.view;
            auto glview = cocos2d::Director::getInstance()->getOpenGLView();
            float factor = glview->getContentScaleFactor();
            float scaleX = glview->getScaleX();
            float scaleY = glview->getScaleY();
            ppc.sourceRect = CGRectMake(
                                        popoverAnchor.x * scaleX / factor,
                                        rootViewController.view.frame.size.height - popoverAnchor.y * scaleY / factor,
                                        0, 0);
            ppc.permittedArrowDirections = UIMenuControllerArrowUp;
        }
    }
    
    [rootViewController presentViewController:activityViewController animated:YES completion:^{
        if(m_listener) {
            ShareResponse resp;
            resp.state = SHARE_STATE_PRESENTED;
            m_listener->onShareState(resp);
        }
    }];
    
    [activityViewController setCompletionWithItemsHandler:^(UIActivityType activityType, BOOL completed, NSArray *returnedItems, NSError *activityError) {
        if(m_listener) {
            ShareResponse resp;
            if(activityError) {
                resp.state = SHARE_STATE_FAIL;
                resp.errorCode = SHARE_ERROR_UNKNOWN;
                if(activityError.localizedDescription)
                    resp.description = [activityError.localizedDescription UTF8String];
            } else {
                if(completed) {
                    resp.state = SHARE_STATE_DONE;
                } else {
                    resp.state = SHARE_STATE_CANCELLED;
                }
                if(activityType) {
                     resp.description = [activityType UTF8String];
                }
            }
            
            m_listener->onShareState(resp);
        }
    }];
    [activityViewController release];
}

NS_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_IOS
