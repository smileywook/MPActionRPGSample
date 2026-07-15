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

## Network Architecture Notes

### Gameplay Framework Role

이 프로젝트에서는 멀티플레이 상태를 다음 기준으로 분리합니다.

- GameMode
  - 서버에만 존재하는 게임 규칙 담당
  - 플레이어 접속 시 초기 PlayerState 설정
  - 예: PostLogin에서 PlayerDisplayName 설정

- GameState
  - 모든 클라이언트가 알아야 하는 게임 전체 상태 담당
  - 접속 중인 PlayerState 목록은 PlayerArray를 통해 확인

- PlayerState
  - 플레이어별 장기 상태 담당
  - Pawn이 죽거나 리스폰되어도 유지되어야 하는 값 저장
  - 예: PlayerDisplayName, 추후 Gold / Score / Team 등

- PlayerController
  - 로컬 플레이어 입력과 UI 소유
  - 로컬 Debug Widget 생성
  - 로컬 PlayerState 이벤트 구독

- Character / Pawn
  - 월드에 존재하는 실제 조작 대상
  - 이동, 전투, HP, 애니메이션 처리 예정
  
- AMPActionRPGSampleCharacter
 - 공격 입력 처리
 - Server RPC 공격 요청
 - 공격 가능 여부 검증
 - 서버 기준 공격 Trace
 - 공격 대상 HealthComponent 탐색
 - 서버 데미지 적용 요청

- UMPHealthComponent
 - CurrentHP / MaxHP 관리
 - 서버 권한 기반 ApplyDamage 처리
 - CurrentHP Replication
 - OnRep_CurrentHP 처리
 - OnHealthChanged Delegate Broadcast
 - bIsDead / OnDeath 처리

- AMPPlayerController
 - 로컬 UI 생성
 - 현재 Pawn의 HealthComponent 이벤트 구독
 - HP 변경 이벤트를 UI에 전달

- UMPNetworkDebugWidget
 - HP Text 표시
 - HealthBar 표시
 - Dead 상태 표시
  
 ### PlayerState Replication Flow

PlayerDisplayName은 PlayerState에 저장하고 서버에서만 변경합니다.

```text
Client Join
→ Server MPGameMode::PostLogin
→ Server gets MPPlayerState
→ Server calls SetPlayerDisplayName
→ PlayerDisplayName changed on server
→ PlayerDisplayName replicated to clients
→ Client OnRep_PlayerDisplayName
→ HandlePlayerDisplayNameChanged
→ OnPlayerDisplayNameChanged Broadcast
→ UI / Controller can react
```

### GameState PlayerArray Flow

GameState는 접속 중인 PlayerState 목록을 PlayerArray로 관리합니다.

```text
Client wants player list
→ Get GameState
→ Cast to MPGameState
→ MPGameState reads PlayerArray
→ Cast each APlayerState to MPPlayerState
→ Read PlayerDisplayName
→ Build player list text
→ WBP_NetworkDebug displays connected players
```

### RepNotify Handling Pattern

복제 변수는 OnRep 함수에서 직접 UI를 수정하지 않습니다.

대신 다음 패턴을 사용합니다.

```text
Server State Change
→ HandleChanged on Server

Client Replication Received
→ OnRep_Variable
→ HandleChanged on Client

HandleChanged
→ Log / Validation / Delegate Broadcast
→ UI or Controller reacts
```

## Test Checklist

### Week 2 Replication Test

#### Listen Server

- [ ] Number of Players = 2
- [ ] Net Mode = Play As Listen Server
- [ ] Listen Server 창에 Network Debug Widget 표시
- [ ] Client 창에 Network Debug Widget 표시
- [ ] 각 창에 PlayerDisplayName 표시
- [ ] 각 창의 Players 목록에 모든 접속자 표시
- [ ] Output Log에서 PostLogin 로그 확인
- [ ] Output Log에서 OnRep_PlayerDisplayName 로그 확인
- [ ] Output Log에서 OnPlayerDisplayNameChanged 이벤트 수신 로그 확인

#### Play As Client

- [ ] Number of Players = 2
- [ ] Net Mode = Play As Client
- [ ] 두 클라이언트 창에 Network Debug Widget 표시
- [ ] 각 클라이언트에 PlayerDisplayName 표시
- [ ] 각 클라이언트의 Players 목록이 동일하게 표시
- [ ] Output Log에서 PlayerState 이벤트 바인딩 로그 확인
- [ ] Output Log에서 PlayerDisplayName 이벤트 수신 로그 확인


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

### Week 2 Day 5 - Replication Flow Review

- 2주차 동안 구현한 PlayerState Replication, GameState PlayerArray, OnRep → Handle → Delegate 구조를 README에 정리했습니다.
- PlayerDisplayName이 서버에서 설정되고 클라이언트로 복제된 뒤, OnRep와 Delegate를 통해 외부 시스템에 변경을 알리는 흐름을 문서화했습니다.
- GameState의 PlayerArray를 통해 모든 접속자의 PlayerState를 확인하고, Debug UI에 접속자 목록을 표시하는 구조를 정리했습니다.
- Listen Server와 Play As Client 환경에서 검증해야 할 체크리스트를 작성했습니다.
- 3주차 HealthComponent 구현 전에 Replication 기본 흐름을 문서와 테스트 기준으로 고정했습니다.

### Week 3 Day 1 - Server Authoritative Health Component

- UMPHealthComponent를 추가했습니다.
- CurrentHP와 MaxHP를 분리하여 HP 데이터를 Component에서 관리하도록 구성했습니다.
- CurrentHP에 ReplicatedUsing=OnRep_CurrentHP를 적용했습니다.
- GetLifetimeReplicatedProps에서 CurrentHP를 DOREPLIFETIME으로 등록했습니다.
- ApplyDamage는 서버 권한에서만 동작하도록 HasAuthority 검사를 추가했습니다.
- SetCurrentHP에서 HP Clamp, 변경 여부 확인, OnHealthChanged Broadcast를 처리하도록 정리했습니다.
- 클라이언트에서는 OnRep_CurrentHP에서 OnHealthChanged를 Broadcast하도록 구성했습니다.
- MPActionRPGSampleCharacter에 HealthComponent를 추가했습니다.
- 이후 PlayerController가 HealthComponent의 OnHealthChanged를 구독하고 UI로 전달하는 흐름으로 확장할 예정입니다.

### Week 3 Day 2 - Bind Health Change Event to PlayerController UI Flow

- MPPlayerController에서 현재 Pawn의 UMPHealthComponent를 찾아 OnHealthChanged 이벤트를 구독하도록 구성했습니다.
- Pawn이 BeginPlay 시점에 준비되지 않을 수 있으므로 BeginPlay, OnPossess, OnRep_Pawn에서 HealthComponent 바인딩을 시도하도록 정리했습니다.
- 기존 HealthComponent 바인딩이 남지 않도록 UnbindHealthComponent를 추가했습니다.
- HandleHealthChanged에서 HP 변경 이벤트를 수신하고 Debug UI로 전달하도록 구성했습니다.
- WBP_NetworkDebug의 C++ 부모 클래스로 UMPNetworkDebugWidget을 추가했습니다.
- UMPNetworkDebugWidget에 SetHealth 함수를 추가하여 HealthText에 현재 HP / 최대 HP를 표시하도록 구현했습니다.
- 테스트용 Exec 명령 TestDamage를 추가하여 클라이언트에서 서버 RPC를 통해 HP 감소를 확인할 수 있도록 구성했습니다.
- HP 변경 흐름이 HealthComponent -> PlayerController -> WBP_NetworkDebug 순서로 전달되는 것을 확인할 예정입니다.

### Week 3 Day 3 - Display Replicated HP with Health Bar

- WBP_NetworkDebug에 HP 표시용 ProgressBar를 추가했습니다.
- ProgressBar 이름을 HealthBar로 지정하고 Is Variable을 체크했습니다.
- UMPNetworkDebugWidget에 HealthBar 바인딩 변수를 추가했습니다.
- SetHealth에서 HealthText와 HealthBar를 함께 갱신하도록 수정했습니다.
- CurrentHP / MaxHP 값을 이용해 HealthBar Percent를 계산하도록 구성했습니다.
- MaxHP가 0 이하일 때 잘못된 나누기가 발생하지 않도록 방어 코드를 추가했습니다.
- 기존 HealthComponent -> PlayerController -> Widget 흐름은 유지하면서 Widget 표시 방식만 확장했습니다.
- TestDamage 명령을 통해 HP Text와 HealthBar가 함께 갱신되는 것을 확인할 예정입니다.

### Week 3 Day 4 - Add Server Authoritative HP Heal Flow

- UMPHealthComponent에 Heal 함수를 추가했습니다.
- Heal은 ApplyDamage와 동일하게 서버 권한에서만 동작하도록 HasAuthority 검사를 적용했습니다.
- HealAmount가 0 이하일 경우 처리하지 않도록 방어 코드를 추가했습니다.
- HP 회복 시에도 CurrentHP를 직접 수정하지 않고 SetCurrentHP를 통해 변경하도록 정리했습니다.
- SetCurrentHP가 데미지와 회복 모두에서 사용하는 공통 HP 변경 함수가 되도록 구조를 정리했습니다.
- MPPlayerController에 테스트용 Exec 명령 TestHeal을 추가했습니다.
- 클라이언트에서 TestHeal을 실행하면 ServerRequestHeal RPC를 통해 서버에서 회복이 적용되도록 구성했습니다.
- HealControlledPawn 함수를 추가하여 현재 Pawn의 UMPHealthComponent에 Heal 요청을 전달하도록 구현했습니다.
- TestDamage와 TestHeal을 함께 사용하여 HP 감소와 회복이 모두 Replication과 OnRep_CurrentHP를 거쳐 UI에 반영되는 것을 확인할 예정입니다.

### Week 3 Day 5 - Add Death State and Death Event Flow

- UMPHealthComponent에 bIsDead 상태 값을 추가했습니다.
- bIsDead에 ReplicatedUsing=OnRep_IsDead를 적용하고 DOREPLIFETIME에 등록했습니다.
- HP가 0이 되었을 때 HandleDeath를 호출하여 사망 상태로 전환하도록 구성했습니다.
- OnDeath Delegate를 추가하여 사망 이벤트를 외부로 알릴 수 있도록 구성했습니다.
- 서버에서는 HandleDeath에서 OnDeath를 Broadcast하고, 클라이언트에서는 OnRep_IsDead에서 OnDeath를 Broadcast하도록 정리했습니다.
- 이미 사망한 상태에서는 ApplyDamage와 Heal이 더 이상 처리되지 않도록 방어 코드를 추가했습니다.
- Heal은 회복, Revive는 부활로 구분하기 위해 죽은 상태에서는 Heal이 동작하지 않도록 구성했습니다.
- WBP_NetworkDebug에 DeathText를 추가하여 State: Alive / State: Dead 상태를 표시하도록 확장했습니다.
- UMPNetworkDebugWidget에 SetDead 함수를 추가했습니다.
- MPPlayerController에서 HealthComponent의 OnDeath 이벤트를 구독하고, 사망 이벤트를 Debug UI에 전달하도록 구성했습니다.
- TestDamage 100 명령으로 HP가 0이 되었을 때 HealthText, HealthBar, DeathText가 함께 갱신되는 것을 확인할 예정입니다.

### Week 4 Day 1 - Server RPC Attack Request Flow

- `IA_Attack` Input Action을 추가했습니다.
- 기존 Input Mapping Context에 공격 입력을 매핑했습니다.
- `AMPActionRPGSampleCharacter`에 `AttackAction`을 추가했습니다.
- 공격 입력 함수 `Attack()`을 추가했습니다.
- 클라이언트가 서버에게 공격을 요청할 수 있도록 `ServerStartAttack` Server RPC를 추가했습니다.
- 서버에서 실제 공격 처리를 시작하는 `HandleAttack()` 함수를 추가했습니다.
- 공격 입력 시 `Attack → ServerStartAttack → ServerStartAttack_Implementation → HandleAttack` 순서로 호출되는 것을 로그로 확인했습니다.
- 이번 단계에서는 데미지, Trace, 피격 판정은 아직 처리하지 않고 서버 공격 처리 진입점만 구성했습니다.
- 이후 단계에서 서버 검증, 공격 Trace, `HealthComponent::ApplyDamage()` 흐름을 연결할 예정입니다.

### Week 4 Day 2 - Server Attack Validation and Attack State

- 서버에서 공격 요청을 바로 처리하지 않고 `CanAttack()`을 통해 공격 가능 여부를 먼저 검사하도록 정리했습니다.
- `AMPActionRPGSampleCharacter`에 `bIsAttacking`, `AttackCooldown`, `AttackDuration`, `LastAttackTime`을 추가했습니다.
- 공격 중복 요청을 막기 위해 `bIsAttacking` 상태를 사용했습니다.
- 공격 연타를 제한하기 위해 `LastAttackTime`과 `AttackCooldown` 기반의 쿨타임 검사를 추가했습니다.
- 사망 상태에서는 공격할 수 없도록 `HealthComponent`의 사망 상태를 확인하도록 구성했습니다.
- 공격이 시작되면 서버에서 `HandleAttack()`이 실행되고, 일정 시간이 지난 뒤 `FinishAttack()`에서 공격 상태를 종료하도록 구성했습니다.
- 공격 요청이 거절될 경우 `AlreadyAttacking`, `Cooldown`, `Dead` 등의 이유를 로그로 확인할 수 있도록 했습니다.
- 이번 단계에서는 아직 Trace나 데미지 적용은 하지 않고, 서버 공격 검증과 기본 공격 상태만 정리했습니다.

### Week 4 Day 3 - Server-side Attack Trace

- 서버 기준으로 공격 판정을 수행하기 위해 `PerformAttackTrace()`를 추가했습니다.
- `AMPActionRPGSampleCharacter`에 `AttackRange`, `AttackTraceRadius`, `bDrawAttackTrace`를 추가했습니다.
- 공격이 시작되면 `HandleAttack()`에서 서버 권한으로 공격 Trace를 실행하도록 구성했습니다.
- `SweepSingleByChannel`과 Sphere Collision Shape를 사용해 캐릭터 전방의 Pawn을 탐색하도록 했습니다.
- Trace 시작 위치는 캐릭터 위치에 `BaseEyeHeight`를 더한 위치로 설정했습니다.
- Trace 종료 위치는 캐릭터 전방 방향에 `AttackRange`를 곱해 계산했습니다.
- `QueryParams.AddIgnoredActor(this)`를 사용해 자기 자신은 공격 판정에서 제외했습니다.
- 공격이 빗나가면 `[Attack][TraceMiss]` 로그를 출력하고, 대상이 감지되면 `[Attack][TraceHit]` 로그를 출력하도록 했습니다.
- Debug Draw를 통해 서버 기준 공격 판정 범위를 시각적으로 확인할 수 있도록 했습니다.
- 이번 단계에서는 아직 데미지를 적용하지 않고, 서버가 공격 대상을 판정하는 구조까지만 구현했습니다.

### Week 4 Day 4 - Server-authoritative Attack Damage

- 서버 Trace 결과로 감지된 대상에게 데미지를 적용하는 흐름을 추가했습니다.
- `AMPActionRPGSampleCharacter`에 `AttackDamage`를 추가했습니다.
- Trace로 감지된 `HitActor`에서 `UMPHealthComponent`를 찾는 `ApplyAttackDamageToActor()` 함수를 추가했습니다.
- 데미지 적용은 서버 권한에서만 실행되도록 `HasAuthority()`를 확인했습니다.
- 자기 자신에게 데미지가 적용되지 않도록 방어 코드를 추가했습니다.
- `HealthComponent`가 없는 대상에게는 데미지를 적용하지 않고 로그만 출력하도록 했습니다.
- 이미 사망한 대상에게는 추가 데미지를 적용하지 않도록 했습니다.
- 공격이 적중하면 서버에서 `HealthComponent::ApplyDamage()`를 호출하도록 연결했습니다.
- HP 변화가 `CurrentHP` Replication, `OnRep_CurrentHP`, `OnHealthChanged` Delegate, `WBP_NetworkDebug` UI 갱신으로 이어지는 것을 확인했습니다.
- Listen Server와 Client 양방향 공격 테스트에서 공격 대상의 HP UI가 감소하는 것을 확인했습니다.

### Week 4 Day 5 - Server RPC Attack Wrap-up

- 4주차에서 구현한 Server RPC 기반 기본 공격 흐름을 최종 정리했습니다.
- 공격 입력이 클라이언트에서 시작되고, Server RPC를 통해 서버 공격 처리로 전달되는 흐름을 정리했습니다.
- 서버에서 `CanAttack()`을 통해 공격 가능 여부를 검증하는 구조를 정리했습니다.
- `bIsAttacking`, `AttackCooldown`, `AttackDuration`, `LastAttackTime`을 사용해 공격 중복 요청과 쿨타임을 제한하는 흐름을 정리했습니다.
- 서버 기준 `Sphere Sweep` Trace를 통해 공격 대상을 판정하는 구조를 정리했습니다.
- Trace 결과로 감지된 대상의 `UMPHealthComponent`를 찾아 서버에서 `ApplyDamage()`를 호출하는 흐름을 정리했습니다.
- HP 변경이 `CurrentHP` Replication, `OnRep_CurrentHP`, `OnHealthChanged` Delegate를 거쳐 UI에 반영되는 흐름을 정리했습니다.
- Listen Server와 Client 양방향 공격 테스트를 통해 서버 권위 공격 흐름을 확인했습니다.
- 허공 공격, 공격 연타, 쿨타임, 사망 상태 공격, 죽은 대상 추가 데미지 방지 케이스를 테스트했습니다.

### Week 5 Day 1 - Hit / Death State Flow

- 5주차 피격 / 사망 / 리스폰 흐름의 첫 단계로, 기존 HealthComponent 기반 피격 및 사망 상태 흐름을 점검했습니다.
- 현재 구조는 이미 CurrentHP, bIsDead, OnHealthChanged, OnDeath 흐름을 가지고 있으므로 전체 구조를 새로 작성하지 않고, 기존 코드 기준으로 필요한 로그를 보강했습니다.
- ApplyDamage에서 서버 권한으로만 데미지를 적용하고, 사망 상태이거나 잘못된 데미지 값일 경우 무시하도록 확인했습니다.
- Heal 역시 서버 권한에서만 동작하며, 사망 상태에서는 일반 Heal로 부활하지 않도록 확인했습니다.
- SetCurrentHP에서 HP Clamp, 변경 여부 확인, OnHealthChanged Broadcast, HP 0 도달 시 HandleDeath 호출 흐름을 다시 확인했습니다.
- HandleDeath에서 bIsDead를 true로 확정하고 OnDeath Delegate를 Broadcast하도록 정리했습니다.
- CurrentHP Replication 이후 OnRep_CurrentHP를 통해 HP UI가 갱신되는 흐름을 확인했습니다.
- bIsDead Replication 이후 OnRep_IsDead를 통해 클라이언트에서도 OnDeath 흐름이 실행되고, DeathText / State: Dead UI가 갱신되는 것을 확인할 예정입니다.
- 죽은 대상에게 추가 데미지가 적용되지 않고, 죽은 캐릭터가 공격할 수 없는 기존 4주차 공격 방어 흐름도 함께 확인했습니다.
- 일반 Heal은 부활 처리가 아니며, 부활은 이후 Respawn 전용 흐름에서 처리할 예정입니다.

### Week 5 Day 2 - Block Input While Dead

- 5주차 2일차에서는 사망 상태를 기준으로 캐릭터 입력 제한 흐름을 정리했습니다.
- 1일차에서 확정한 bIsDead 상태를 입력 처리에도 사용하여, 죽은 캐릭터가 이동 / 점프 / 공격을 수행하지 못하도록 구성했습니다.
- 기존 JumpAction이 ACharacter::Jump에 직접 바인딩되어 있어 사망 상태를 검사하기 어려웠기 때문에, StartJump / StopJump 함수를 추가하고 JumpAction 바인딩을 해당 함수로 변경했습니다.
- Move 함수 시작 지점에서 IsCharacterDead를 확인하여 사망 상태에서는 AddMovementInput이 호출되지 않도록 했습니다.
- Attack 함수에서도 IsCharacterDead를 먼저 확인하여 사망 상태에서는 ServerStartAttack RPC를 보내지 않도록 정리했습니다.
- 단, 서버 권위 검증을 유지하기 위해 CanAttack의 Dead 검사는 그대로 남겨두었습니다.
- 이를 통해 클라이언트 입력 단계와 서버 검증 단계 양쪽에서 사망 상태 입력이 제한되도록 구성했습니다.
- Look 입력은 오늘 범위에서는 제한하지 않고, 죽은 상태에서도 카메라 확인은 가능하도록 유지했습니다.

### Week 5 Day 3 - Respawn Request Server RPC

- 5주차 3일차에서는 사망한 플레이어가 서버에 리스폰을 요청하는 기본 RPC 흐름을 추가했습니다.
- MPPlayerController에 TestRespawn Exec 명령을 추가하여 로컬에서 리스폰 요청을 테스트할 수 있도록 했습니다.
- 클라이언트에서 TestRespawn을 실행하면 ServerRequestRespawn RPC를 통해 서버로 요청을 전달하도록 구성했습니다.
- Listen Server처럼 이미 서버 권한을 가진 경우에는 RPC를 거치지 않고 바로 서버 검증 함수로 들어가도록 처리했습니다.
- 서버에서는 RequestRespawnControlledPawn에서 현재 Pawn과 HealthComponent를 확인하고, 사망 상태인지 검증했습니다.
- 살아있는 상태에서 리스폰을 요청하면 Reason=NotDead로 거절되도록 했습니다.
- 사망 상태에서 리스폰을 요청하면 Respawn Accepted 로그가 출력되도록 했습니다.
- 오늘은 리스폰 요청 흐름만 검증하고, 실제 HP 초기화 / 위치 재배치 / UI 복구는 다음 일차에서 처리할 예정입니다.

### Week 5 Day 4 - Server Respawn Process

- 5주차 4일차에서는 3일차에서 만든 리스폰 요청 흐름에 실제 서버 리스폰 처리를 연결했습니다.
- HealthComponent에 OnRespawn Delegate와 ResetForRespawn 함수를 추가했습니다.
- ResetForRespawn에서는 서버 권한에서만 bIsDead를 false로 되돌리고, CurrentHP를 MaxHP로 복구하도록 구성했습니다.
- OnRep_IsDead에서 bIsDead가 true로 복제되면 OnDeath를 Broadcast하고, false로 복제되면 OnRespawn을 Broadcast하도록 수정했습니다.
- PlayerController는 OnRespawn 이벤트를 구독하고, HandleRespawn에서 Health UI를 갱신하고 Dead 상태 UI를 해제하도록 했습니다.
- RequestRespawnControlledPawn에서 사망 상태 검증을 통과하면 RespawnControlledPawn을 호출하도록 변경했습니다.
- RespawnControlledPawn에서는 서버 권한으로 Pawn을 PlayerStart 위치로 이동시키고, HealthComponent의 ResetForRespawn을 호출했습니다.
- 이를 통해 Client는 리스폰을 요청만 하고, HP 복구 / 사망 상태 해제 / 위치 재배치는 모두 서버에서 처리되도록 구성했습니다.
- 리스폰 후 HealthText / HealthBar가 MaxHP로 복구되고, DeathText 또는 State: Dead UI가 해제되며, 이동 / 점프 / 공격 입력이 다시 가능한 것을 확인했습니다.

### Week 5 Day 5 - Hit / Death / Respawn Flow Summary

- 5주차 5일차에서는 피격 / 사망 / 리스폰 흐름을 최종 정리했습니다.
- 1일차에서는 HealthComponent 기반 피격 및 사망 상태 흐름을 점검하고, ApplyDamage / Heal / HP 변경 / 사망 / OnRep 로그를 보강했습니다.
- 2일차에서는 bIsDead 상태를 기준으로 사망 중 이동 / 점프 / 공격 입력을 제한했습니다.
- 3일차에서는 MPPlayerController에 TestRespawn Exec 명령과 ServerRequestRespawn Server RPC를 추가하여, 클라이언트가 서버에 리스폰을 요청하는 흐름을 만들었습니다.
- 4일차에서는 서버에서 실제 리스폰 처리를 수행하도록 연결했습니다. 서버는 PlayerStart 위치를 찾아 Pawn을 이동시키고, HealthComponent::ResetForRespawn을 통해 CurrentHP를 MaxHP로 복구하고 bIsDead를 false로 되돌립니다.
- HealthComponent에는 OnRespawn Delegate를 추가하여, bIsDead가 false로 복제되었을 때 클라이언트에서도 UI가 Alive 상태로 복구되도록 했습니다.
- PlayerController는 OnHealthChanged, OnDeath, OnRespawn을 구독하고, HealthText / HealthBar / DeathText UI를 갱신합니다.
- 이번 주차를 통해 피격 → 사망 → 입력 제한 → 리스폰 요청 → 서버 리스폰 처리 → UI 복구 흐름을 하나의 멀티플레이 패턴으로 정리했습니다.

## Week 6 Day 1 - SkillComponent 기본 구조 추가

- 스킬 기능을 Character에서 분리하기 위한 `UMPSkillComponent`를 추가했다.
- 기존 기본 공격은 Character에 유지하고, 스킬 사용 요청 / 서버 검증 / 쿨다운 / 판정 흐름은 SkillComponent에서 담당하도록 구조를 분리할 예정이다.
- `UMPSkillComponent`는 ActorComponent 기반으로 생성했으며, 이후 서버 권위 스킬 상태를 다룰 수 있도록 `SetIsReplicatedByDefault(true)`를 설정했다.
- Character에 SkillComponent를 추가하고 `CreateDefaultSubobject`로 생성되도록 연결했다.
- Listen Server / Client 실행 시 각 Pawn에서 SkillComponent BeginPlay 로그가 출력되는 것을 확인했다.
- 오늘은 실제 스킬 입력, 데미지, 쿨다운은 구현하지 않고 6주차 스킬 시스템의 기반 구조만 준비했다.