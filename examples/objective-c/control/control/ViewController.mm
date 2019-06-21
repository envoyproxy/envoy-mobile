#import <UIKit/UIKit.h>
#import "ViewController.h"

#pragma mark - Constants

NSString* _CELL_ID = @"cell-id";
NSString* _ENDPOINT = @"https://s3.amazonaws.com/api.lyft.com/static/demo/hello_world.txt";

#pragma mark - ViewController

@interface ViewController ()
@property (nonatomic, weak) NSTimer* requestTimer;
@property (nonatomic, assign) int requestCount;
@end

@implementation ViewController

#pragma mark - Lifecycle

- (instancetype)init {
    self = [super init];
    if (self) {
        self.tableView.allowsSelection = FALSE;
    }
    return self;
}

- (void)dealloc {
    [self.requestTimer invalidate];
    self.requestTimer = nil;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    [self startRequests];

    [self setTitle:@"No requests"];
}

#pragma mark - Requests

- (void)startRequests {
    // Note that the first delay will give Envoy time to start up.
    self.requestTimer = [NSTimer scheduledTimerWithTimeInterval:0.2
                                                         target:self
                                                       selector:@selector(performRequest)
                                                       userInfo:nil
                                                        repeats:true];
}

- (void)performRequest {
    NSURLSession* session = [NSURLSession sharedSession];
    // Note that the request is sent to the envoy thread listening locally on port 9001.
    NSURL* url = [NSURL URLWithString:_ENDPOINT];
    NSURLRequest* request = [NSURLRequest requestWithURL:url];
    NSLog(@"Starting request to '%@'", url.path);

    __weak ViewController* weakSelf = self;
    NSURLSessionDataTask* task =
    [session dataTaskWithRequest:request
               completionHandler:^(NSData* data, NSURLResponse* response, NSError* error) {
                   if (error == nil && [(NSHTTPURLResponse*)response statusCode] == 200) {
                       [weakSelf handleResponse:(NSHTTPURLResponse*)response data:data];
                   } else {
                       NSLog(@"Received error: %@", error);
                   }
               }];
    [task resume];
}

- (void)handleResponse:(NSHTTPURLResponse*)response data:(NSData*)data {
    NSString* body = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
    if (body == nil || response == nil) {
        NSLog(@"Failed to deserialize response string");
        return;
    }

    NSLog(@"Response:\n%ld bytes\n%@\n%@", data.length, body, [response allHeaderFields]);
    BOOL isEnvoy = [[[response allHeaderFields] valueForKey:@"Server"] isEqualToString:@"envoy"];
    dispatch_async(dispatch_get_main_queue(), ^{
        self.requestCount++;
        [self setTitle:[NSString stringWithFormat:@"%i successes%@",
                        self.requestCount, isEnvoy ? @" from Envoy" : @""]];

//        [self.responses addObject:value];
//        [self.tableView reloadData];
    });
}

#pragma mark - UITableView

- (NSInteger)numberOfSectionsInTableView:(UITableView*)tableView {
    return 1;
}

- (NSInteger)tableView:(UITableView*)tableView numberOfRowsInSection:(NSInteger)section {
    return 0;//self.responses.count;
}

- (UITableViewCell*)tableView:(UITableView*)tableView
        cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:_CELL_ID];
    if (cell == nil) {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleSubtitle
                                      reuseIdentifier:_CELL_ID];
    }
    return cell;
}

@end
