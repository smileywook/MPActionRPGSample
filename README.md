# MPActionRPGSample

UE5 기반 멀티플레이 액션 RPG 미니 샘플 프로젝트입니다.

## Project Goal

이 프로젝트는 완성형 게임 제작이 아니라, 클라이언트 프로그래머 포트폴리오를 목적으로 합니다.

핵심 목표는 다음과 같습니다.

- Dedicated Server 기반 멀티플레이 실행 환경 구성
- Client 2개 접속 검증
- 서버 권위 기반 전투 판정
- HP / 사망 / 리스폰 상태 Replication
- 클라이언트 입력과 서버 판정 흐름 분리
- Replicated State 기반 UI 갱신
- 네트워크 지연 상황에서의 상태 불일치 방지 실험

## Current Scope

1차 목표는 다음 기능까지로 제한합니다.

- 2인 접속
- 이동 동기화
- 기본 공격
- HP Replication
- 사망 / 리스폰
- 몬스터 1종
- 스킬 1개
- HUD 표시
- 테스트 시나리오 문서화

## Out of Scope

초기 버전에서는 아래 기능을 제외합니다.

- 인벤토리
- 장비
- 퀘스트
- 상점
- 로그인 서버
- 매치메이킹
- 대규모 오픈월드
- 복잡한 GAS 구조
- Replication Graph
- Iris

## Dev Log

### Week 1 Day 1 - Project Setup

- UE5 C++ Third Person 기반 프로젝트를 생성했습니다.
- 멀티플레이 액션 RPG 샘플의 1차 목표와 제외 범위를 정리했습니다.
- 초기 단계에서는 전투 구현보다 서버/클라이언트 실행 환경과 네트워크 디버그 표시를 우선 구현하기로 했습니다.

### Week 1 Day 2 - PIE Multiplayer Test

- Unreal Editor의 Play In Editor 멀티플레이 옵션을 확인했습니다.
- Number of Players를 2로 설정하고 Listen Server 모드에서 두 플레이어가 같은 맵에 접속하는 것을 확인했습니다.
- Play As Client 모드에서 백그라운드 서버와 2개의 클라이언트 창이 실행되는 흐름을 확인했습니다.
- Listen Server는 서버 역할을 겸하는 플레이어 창이 존재하고, Play As Client는 Dedicated Server에 가까운 구조로 클라이언트 창만 표시된다는 차이를 정리했습니다.
- 이후 네트워크 디버그 UI에서 NetMode / LocalRole / RemoteRole을 표시하기로 했습니다.

### Week 1 Day 3 - Gameplay Framework Base Classes

- 멀티플레이 샘플에서 사용할 기본 Gameplay Framework 클래스를 생성했습니다.
- MPGameMode, MPGameState, MPPlayerState, MPPlayerController를 추가했습니다.
- MPGameMode에서 GameStateClass, PlayerControllerClass, PlayerStateClass, DefaultPawnClass를 명시적으로 지정했습니다.
- GameMode는 서버 전용 게임 규칙, GameState는 전체 게임 상태, PlayerState는 플레이어별 상태, PlayerController는 입력과 UI 소유 역할로 분리하기로 했습니다.
- PIE 멀티플레이 실행에서 기존 Third Person Character가 정상적으로 스폰되고 이동 동기화되는 것을 확인했습니다.

### Week 1 Day 4 - Network Role Logging

- Character와 PlayerController의 BeginPlay에서 NetMode / LocalRole / RemoteRole / HasAuthority / IsLocallyControlled 상태를 로그로 출력했습니다.
- Listen Server와 Play As Client 실행 모드에서 서버와 클라이언트의 Role 차이를 확인했습니다.
- 서버 월드에서는 복제 Actor가 Authority를 가지며, 클라이언트 월드에서는 로컬 플레이어 Pawn이 AutonomousProxy, 다른 플레이어 Pawn이 SimulatedProxy로 동작하는 것을 확인했습니다.
- 이후 전투 판정과 HP 변경은 HasAuthority 기준으로 서버에서만 처리하도록 설계할 예정입니다.

### Week 1 Day 5 - Network Debug Widget

- 네트워크 상태를 화면에서 확인하기 위한 WBP_NetworkDebug 위젯을 생성했습니다.
- MPPlayerController에서 로컬 컨트롤러일 때만 Debug Widget을 생성하고 Viewport에 추가하도록 구현했습니다.
- 위젯에는 NetMode, LocalRole, RemoteRole, HasAuthority, IsLocallyControlled, Pawn Name을 표시했습니다.
- Listen Server와 Play As Client 실행 모드에서 각 클라이언트 창의 Role 차이를 화면에서 확인했습니다.
- C++는 기본 로직을 담당하고, Blueprint는 Widget Class와 에셋 참조 설정을 담당하도록 분리했습니다.

### Week 2 Day 1 - PlayerState Replication Basics

- MPPlayerState에 PlayerDisplayName을 추가하고 ReplicatedUsing 방식으로 복제되도록 구현했습니다.
- GetLifetimeReplicatedProps에서 PlayerDisplayName을 DOREPLIFETIME으로 등록했습니다.
- MPGameMode::PostLogin에서 서버가 접속한 플레이어의 표시 이름을 설정하도록 구현했습니다.
- 서버에서 변경한 PlayerDisplayName이 클라이언트로 복제되고, 클라이언트에서 OnRep_PlayerDisplayName이 호출되는 흐름을 로그로 확인했습니다.
- 플레이어별 네트워크 상태는 Character가 아니라 PlayerState에 두는 것이 적절하다는 기준을 정리했습니다.

### Week 2 Day 2 - Display PlayerState on Debug UI

- WBP_NetworkDebug에 PlayerDisplayName 표시 항목을 추가했습니다.
- MPPlayerState의 GetPlayerDisplayName 함수를 BlueprintPure로 노출했습니다.
- Debug Widget에서 Owning Player의 PlayerState를 가져와 MPPlayerState로 Cast한 뒤, 복제된 PlayerDisplayName을 화면에 표시했습니다.
- Listen Server와 Play As Client 실행 모드에서 각 클라이언트 화면에 서버에서 설정한 PlayerDisplayName이 표시되는 것을 확인했습니다.
- 플레이어별 장기 상태는 PlayerState에 두고, UI는 Replicated PlayerState를 읽어 표시하는 흐름을 확인했습니다.

### Week 2 Day 3 - GameState PlayerArray and Player List UI

- MPGameState에서 GameState의 PlayerArray를 순회해 접속 중인 PlayerState 목록을 Debug UI에 표시하도록 구현했습니다.
- MPGameState에 GetPlayerListText와 GetConnectedPlayerCount 함수를 BlueprintPure로 노출했습니다.
- WBP_NetworkDebug에서 Get Game State → Cast To MPGameState 흐름으로 GameState에 접근하고, 접속자 목록을 화면에 표시했습니다.
- Listen Server와 Play As Client 실행 모드에서 모든 클라이언트가 동일한 접속자 목록을 확인할 수 있음을 검증했습니다.
- GameState는 게임 전체 공유 상태, PlayerState는 플레이어별 상태를 담당한다는 기준을 정리했습니다.

### Week 2 Day 4 - PlayerState Change Event Pattern

- MPPlayerState에 PlayerDisplayName 변경을 알리는 OnPlayerDisplayNameChanged Delegate를 추가했습니다.
- SetPlayerDisplayName과 OnRep_PlayerDisplayName에서 공통 HandlePlayerDisplayNameChanged 함수를 호출하도록 정리했습니다.
- HandlePlayerDisplayNameChanged에서 로그 출력과 Delegate Broadcast를 함께 처리하도록 구성했습니다.
- MPPlayerController에서 로컬 PlayerState의 OnPlayerDisplayNameChanged 이벤트를 구독하고, 이벤트 수신 로그를 확인했습니다.
- PlayerState가 BeginPlay 시점에 아직 준비되지 않을 수 있으므로 BeginPlay와 OnRep_PlayerState 양쪽에서 바인딩을 시도하도록 처리했습니다.
- 이후 HP / Gold / Skill Cooldown UI도 OnRep → Handle → Delegate → UI 갱신 패턴으로 확장할 예정입니다.