#import <Envoy/Envoy-Swift.h>
#import <UIKit/UIKit.h>
#import "Result.h"
#import "ViewController.h"

#pragma mark - Constants

NSString *_CELL_ID = @"cell-id";
NSString *_REQUEST_AUTHORITY = @"api.lyft.com";
NSString *_REQUEST_PATH = @"/ping";
NSString *_REQUEST_SCHEME = @"https";

#pragma mark - ViewController

@interface ViewController ()
@property (nonatomic, strong) id<StreamClient> client;
@property (nonatomic, strong) NSMutableArray<Result *> *results;
@property (nonatomic, weak) NSTimer *requestTimer;
@end

@implementation ViewController

#pragma mark - Lifecycle

- (instancetype)init {
  self = [super init];
  if (self) {
    self.results = [NSMutableArray new];
    self.tableView.allowsSelection = NO;
    [self startEnvoy];
  }
  return self;
}

- (void)startEnvoy {
  NSLog(@"starting Envoy...");
  NSError *error;
  EngineBuilder *builder = [[EngineBuilder alloc] init];
  id<Engine> engine = [builder buildAndReturnError:&error];
  if (error) {
    NSLog(@"starting Envoy failed: %@", error);
  } else {
    NSLog(@"started Envoy, beginning requests...");
    self.client = [engine getStreamClient];
    [self startRequests];
  }
}

- (void)dealloc {
  [self.requestTimer invalidate];
  self.requestTimer = nil;
}

#pragma mark - Requests

- (void)startRequests {
  self.requestTimer = [NSTimer scheduledTimerWithTimeInterval:1.0
                                                       target:self
                                                     selector:@selector(performRequest)
                                                     userInfo:nil
                                                      repeats:YES];
}

- (void)performRequest {
  NSLog(@"starting request to '%@'", _REQUEST_PATH);

  // Note: this request will use an http/1.1 stream for the upstream request.
  // The Swift example uses h2. This is done on purpose to test both paths in end-to-end tests
  // in CI.
  RequestHeadersBuilder *builder = [[RequestHeadersBuilder alloc] initWithMethod:RequestMethodGet
                                                                          scheme:_REQUEST_SCHEME
                                                                       authority:_REQUEST_AUTHORITY
                                                                            path:_REQUEST_PATH];
  [builder addUpstreamHttpProtocol:UpstreamHttpProtocolHttp1];
  RequestHeaders *headers = [builder build];

  __weak ViewController *weakSelf = self;
  StreamPrototype *prototype = [self.client newStreamPrototype];
  [prototype setOnResponseHeadersWithClosure:^(ResponseHeaders *headers, BOOL endStream) {
    int statusCode = [[[headers valueForName:@":status"] firstObject] intValue];
    NSString *message = [NSString stringWithFormat:@"received headers with status %i", statusCode];
    NSLog(@"%@", message);
    [weakSelf addResponseMessage:message
                    serverHeader:[[headers valueForName:@"server"] firstObject]
                           error:nil];
  }];
  [prototype setOnErrorWithClosure:^(EnvoyError *error) {
    // TODO: expose attemptCount. https://github.com/lyft/envoy-mobile/issues/823
    NSString *message =
        [NSString stringWithFormat:@"failed within Envoy library %@", error.message];
    NSLog(@"%@", message);
    [weakSelf addResponseMessage:message serverHeader:nil error:message];
  }];

  Stream *stream = [prototype startWithQueue:dispatch_get_main_queue()];
  [stream sendHeaders:headers endStream:YES];
}

- (void)addResponseMessage:(NSString *)message
              serverHeader:(NSString *)serverHeader
                     error:(NSString *)error {
  Result *result = [Result new];
  result.message = message;
  result.serverHeader = serverHeader;
  result.error = error;

  [self.results insertObject:result atIndex:0];
  [self.tableView reloadData];
}

#pragma mark - UITableView

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
  return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
  return self.results.count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView
         cellForRowAtIndexPath:(NSIndexPath *)indexPath {
  UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:_CELL_ID];
  if (cell == nil) {
    cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleSubtitle
                                  reuseIdentifier:_CELL_ID];
  }

  Result *result = self.results[indexPath.row];
  if (result.error == nil) {
    cell.textLabel.text = result.message;
    cell.detailTextLabel.text =
        [NSString stringWithFormat:@"'server' header: %@", result.serverHeader];

    cell.textLabel.textColor = [UIColor blackColor];
    cell.detailTextLabel.textColor = [UIColor blackColor];
    cell.contentView.backgroundColor = [UIColor whiteColor];
  } else {
    cell.textLabel.text = result.error;
    cell.detailTextLabel.text = nil;

    cell.textLabel.textColor = [UIColor whiteColor];
    cell.detailTextLabel.textColor = [UIColor whiteColor];
    cell.contentView.backgroundColor = [UIColor redColor];
  }

  return cell;
}

@end
